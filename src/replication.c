//
//  replication.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/12/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include "data_structs.h"
#include "replication.h"
#include "exlog.h"
#include "files.h"

static volatile int           s_replication_min_redundancy = 3;
static volatile int           s_replication_overredundant_perc_thresh = 3;

static TieredPriorityQueue_t *s_replication_queue = NULL;
static volatile int           s_replication_queue_paused = 0;
static pthread_t              s_replication_thread;

void *replication_thread(void *arg);

void replication_task_init(ReplicationTask_t *task) {
	task->path[0] = 0;
	task->volume_basepath[0] = 0;
}

/* ------------------------- Replication Interruption ----------------------------- */

static volatile int           s_replication_halt_replication_of_file = 0;
static char                   s_replication_halt_replication_of_file_path[PATH_MAX];
static char                   s_replication_currently_replicating_file[PATH_MAX];
static pthread_mutex_t        s_replication_halt_replication_mutex = PTHREAD_MUTEX_INITIALIZER;

void replication_halt_replication_of_file(const char *path) {
	pthread_mutex_lock(&s_replication_halt_replication_mutex);
	if (strncmp(path, s_replication_currently_replicating_file, PATH_MAX)) {
		strncpy(s_replication_halt_replication_of_file_path, path, PATH_MAX);
		s_replication_halt_replication_of_file = 1;
	}
	pthread_mutex_unlock(&s_replication_halt_replication_mutex);	
}

/* ------------------------- Replication Settings --------------------------------- */

void replication_set_min_redundancy_count(int min_redundancy) {
	s_replication_min_redundancy = min_redundancy;
}

int  replication_get_min_redundancy_count() {
	return s_replication_min_redundancy;
}

void replication_set_overredundant_removal_perc_thresh(int removal_thresh) {
	s_replication_overredundant_perc_thresh = removal_thresh;
}

int  replication_get_overredundant_removal_perc_thresh() {
	return s_replication_overredundant_perc_thresh;
}


/* ------------------------- Replication Start/Pause ------------------------------ */

void replication_start() {
	
	/* Unpause replication thread */
	s_replication_queue_paused = 0;
	
	if (!s_replication_queue) {
		s_replication_queue = tiered_priority_queue_create();
		pthread_create(&s_replication_thread, NULL, replication_thread, NULL);
	}
	
}

void replication_pause() {
	/* Signal replication thread to pause */
	s_replication_queue_paused = 1;
}

void replication_queue_task(ReplicationTask_t *task, OperationPriority_t priority, int front) {
	ReplicationTask_t *task_to_queue = (ReplicationTask_t*)malloc(sizeof(ReplicationTask_t));
	memcpy(task_to_queue, task, sizeof(ReplicationTask_t));
	
	tiered_priority_queue_push(s_replication_queue, priority, front, task_to_queue);
}


/* ---------------------------- Replication Helpers ---------------------------------- */

/* Add a REP_OP_VERIFY_VOLUME_DIRS task for a given volume */
static inline void __replication_queue_volume_check(RaidVolume_t *volume) {
	ReplicationTask_t task;
	replication_task_init(&task);
	task.opcode = REP_OP_VERIFY_VOLUME_DIRS;
	strncpy(task.volume_basepath, volume->basepath, PATH_MAX);
	replication_queue_task(&task, OP_PRI_CRITICAL, 1);
}

/* For regular file, we're going to:
 * Find file with latest modified time
 * copy it to work directory on target
 * move it to proper raid location
 * update file times to match original
 */
static int __replication_copy_regular_file(const char *relative_path, RaidVolume_t *to_vol) {
	
	EXLog(REPL, DBG, " > __replication_copy_regular_file [%s > %s]", relative_path, to_vol->basepath);
	
	RaidVolume_t *from_vol;
	char fullpath_from[PATH_MAX];
	struct stat stbuf;
	
	/* Find volume w/ latest modified time */
	int not_found = volume_most_recently_modified_instance(relative_path, &from_vol, fullpath_from, &stbuf);
	if (not_found || ((stbuf.st_mode & S_IFMT) != S_IFREG)) {
		EXLog(REPL, DBG, "   > Path is not a regular file");
		return 0;
	}
	
	/* Get target work path */
	char workpath[PATH_MAX];
	if (!volume_avaialble_work_path(to_vol, workpath)) {
		EXLog(REPL, DBG, "   > Volume does not have an available work path");
		__replication_queue_volume_check(to_vol);
		return 0;
	}
	
	/* Copy source -> dest */
	#define REGFILE_COPY_CHUNK_SIZE 32768
	char copy_buffer[REGFILE_COPY_CHUNK_SIZE];
	
	int from_fd = open(fullpath_from, O_RDONLY | O_SHLOCK);
	if (from_fd < 0) {
		EXLog(REPL, DBG, "   > Could not open path for reading [%s]", fullpath_from);
		__replication_queue_volume_check(from_vol);
		return 0;
	}
	
	int to_fd = open(workpath, O_CREAT | O_WRONLY | O_APPEND, stbuf.st_mode);
	if (to_fd < 0) {
		close(from_fd);
		EXLog(REPL, DBG, "   > Could not open path for writing [%s]", workpath);
		__replication_queue_volume_check(to_vol);
		return 0;
	}

	/* Both files are open */
	while (1) {
		ssize_t bytes_read = read(from_fd, copy_buffer, (size_t)REGFILE_COPY_CHUNK_SIZE);
		if (bytes_read < 0) {
			/* Shit, there was an error.  Shut it all down */
			close(from_fd);
			close(to_fd);
			unlink(workpath);
			EXLog(REPL, DBG, "   > Read error on path [%s]", fullpath_from);
			__replication_queue_volume_check(from_vol);
			return 0;
		}
		
		if (bytes_read == 0) {
			/* We hit the end of the file, wrap it up! */
			close(from_fd);
			close(to_fd);
			char topath[PATH_MAX];
			volume_full_path_for_raid_path(to_vol, relative_path, topath);
			rename(workpath, topath);
			
			struct timeval _times[2];
			_times[0].tv_sec = stbuf.st_atimespec.tv_sec; _times[0].tv_usec = 0;
			_times[1].tv_sec = stbuf.st_mtimespec.tv_sec; _times[1].tv_usec = 0;
			
			utimes(topath, _times);			
			return 1;
		}
		
		/* Otherwise write to workpath */
		ssize_t bytes_left = bytes_read;
		char *bufptr = copy_buffer;
		while (bytes_left) {
			/* Check for halting */
			if (s_replication_halt_replication_of_file) {
				s_replication_halt_replication_of_file = 0;
				pthread_mutex_lock(&s_replication_halt_replication_mutex);
				if (!strncmp(s_replication_halt_replication_of_file_path, relative_path, PATH_MAX)) {
					/* Shut it down! */
					pthread_mutex_unlock(&s_replication_halt_replication_mutex);
					close(from_fd);
					close(to_fd);
					EXLog(REPL, DBG, "   > Shutting down because path [%s] was opened by the OS", workpath);
					return 0;
				}
				pthread_mutex_unlock(&s_replication_halt_replication_mutex);
			}
			
			ssize_t bytes_written = write(to_fd, bufptr, (size_t)bytes_left);
			if (bytes_written < 0) {
				/* Shut it all down! */
				close(from_fd);
				close(to_fd);
				unlink(workpath);
				EXLog(REPL, DBG, "   > Write error on path [%s]", workpath);
				__replication_queue_volume_check(to_vol);
				return 0;
			}
			
			/* Otherwise ok */
			bytes_left -= bytes_written;
			bufptr += bytes_written;
		}		
	}
	
	return 0;
}

/* ---------------------------- Replication Operations ------------------------------- */

void replication_task_mirror_directory(ReplicationTask_t *task) {
	
	int    absences      = 0;
	mode_t mode          = 0;
	int    mode_conflict = 0;
	
	EXLog(REPL, DBG, "Task: mirror directory [%s]", task->path);
	
	/* Sanity */
	if (!task->path) return;
	
	/* Perform diagnostics on the path */
	volume_diagnose_raid_file_posession(task->path, NULL, &absences, &mode, &mode_conflict, NULL, NULL, NULL, NULL);
	
	/* If this isn't a directory, just exit */
	if (!mode_conflict && (mode & S_IFMT) != S_IFDIR) {
		EXLog(REPL, DBG, " > Path is not a directory");
		return;
	}
	
	/* Conflict?  Resolve */
	if (mode_conflict) {
		EXLog(REPL, DBG, " > Detected mode conflicts");
		volume_resolve_conflicting_modes(task->path);
		
		/* Perform diagnostics on the path again */
		volume_diagnose_raid_file_posession(task->path, NULL, &absences, &mode, &mode_conflict, NULL, NULL, NULL, NULL);
		
		/* If this isn't a directory, just exit */
		if (!mode_conflict && (mode & S_IFMT) != S_IFDIR) {
			EXLog(REPL, DBG, " > Path is not a directory [after conflict resolution]");
			return;
		}
	}
	
	/* Absences? Make directories */
	if (absences) {
		EXLog(REPL, DBG, " > Absences found; mirroring");
		volume_mkdir_path_on_active_volumes(task->path, (mode & 0xFFFF) | S_IFDIR);
	}
	
	/* Now we should queue up child directories */
	/* Get all the DIR entries for the active volumes */
	DIR **entries = volume_active_dir_entries(task->path);
	DIR **entries_start = entries;
	
	ReplicationTask_t child_task;
	replication_task_init(&child_task);
	child_task.opcode = REP_OP_MIRROR_DIRECTORY;
	child_task.volume_basepath[0] = 0;
	
	if (entries) {
		/* create a hash table that will help detect duplicate entries */
		Dictionary_t *dic = dictionary_create_with_size(512);
		int64_t tmp;
		while (*entries) {
			/* For each DIR, iterate through and grab entries */
			DIR *entry = *entries;
			struct dirent *dent = readdir(entry);
			while (dent) {
				
				if (dent->d_type == DT_DIR && strcmp("..", dent->d_name) && strcmp(".", dent->d_name)) {
					/* For each entry, add it to the list if it doesn't exist in the hash table */
					if (!dictionary_get_int(dic, dent->d_name, &tmp)) {
						dictionary_set_int(dic, dent->d_name, tmp);
						snprintf(child_task.path, PATH_MAX-1, "%s/%s", task->path, dent->d_name);
						replication_queue_task(&child_task, OP_PRI_SYNC_DIR, 1);
					}
				}
				
				dent = readdir(entry);
			}
			closedir(entry); /* Don't forget to close the DIR after use! */
			entries++;
		}
		/* Free memory */
		dictionary_destroy(dic);
		free(entries_start);
	}
	
}

void replication_task_balance_files_in_dir(ReplicationTask_t *task) {

	EXLog(REPL, DBG, "Task: Balance files in directory [%s]", task->path);

	/* Sanity */
	if (!task->path) return;
	
	/* Strategy:
		* Iterate through directory
			* For each file, check conflicts
			* If no conflicts exist, add task to balance it to front of queue
		* If conflicts exists, put task to balance directory back on the front of the task list
		* If no conflicts existed, iterate through directory again
			* Put each subdir up for balancing
	 */
	
	/* Get all the DIR entries for the active volumes */
	DIR **entries = volume_active_dir_entries(task->path);
	DIR **entries_start = entries;
	
	ReplicationTask_t child_task;
	replication_task_init(&child_task);
	child_task.opcode = REP_OP_BALANCE_FILE;
	child_task.volume_basepath[0] = 0;
	
	int saw_conflict = 0;
	
	if (entries) {
		/* create a hash table that will help detect duplicate entries */
		Dictionary_t *dic = dictionary_create_with_size(512);
		int64_t tmp;
		while (*entries) {
			/* For each DIR, iterate through and grab entries */
			DIR *entry = *entries;
			struct dirent *dent = readdir(entry);
			while (dent) {
				
				if (dent->d_type != DT_DIR && strcmp("..", dent->d_name) && strcmp(".", dent->d_name)) {
					/* For each entry, add it to the list if it doesn't exist in the hash table */
					if (!dictionary_get_int(dic, dent->d_name, &tmp)) {
						dictionary_set_int(dic, dent->d_name, tmp);
						
						snprintf(child_task.path, PATH_MAX-1, "%s/%s", task->path, dent->d_name);
						
						mode_t mode = 0;
						int    mode_conflict = 0;
						
						/* Perform diagnostics on the path */
						volume_diagnose_raid_file_posession(child_task.path, NULL, NULL, &mode, &mode_conflict, NULL, NULL, NULL, NULL);
						
						if (!mode_conflict) {
							/* No conflict, add task for it */
							replication_queue_task(&child_task, OP_PRI_SYNC_FILE, 1);
						} else {
							/* Conflict? Resolve it */
							EXLog(REPL, DBG, " > Conflict detected on file [%s]", child_task.path);
							volume_resolve_conflicting_modes(child_task.path);
							saw_conflict = 1;
						}
					}
				}
				
				dent = readdir(entry);
			}
			closedir(entry); /* Don't forget to close the DIR after use! */
			entries++;
		}
		/* Free memory */
		dictionary_destroy(dic);
		free(entries_start);
	}
	
	/* Now, did we see a conflict?  If so, re-queue the current tast */
	if (saw_conflict) {
		EXLog(REPL, DBG, " > Conflicts detected; restarting sync");
		replication_queue_task(task, OP_PRI_SYNC_FILE, 1);
		return;
	}
	
	/* Otherwise, go through the directory and queue up balancing for all subdirectories */
	entries = volume_active_dir_entries(task->path);
	entries_start = entries;
	
	replication_task_init(&child_task);
	child_task.opcode = REP_OP_BALANCE_FILES_IN_DIR;
	child_task.volume_basepath[0] = 0;
	
	if (entries) {
		/* create a hash table that will help detect duplicate entries */
		Dictionary_t *dic = dictionary_create_with_size(512);
		int64_t tmp;
		while (*entries) {
			/* For each DIR, iterate through and grab entries */
			DIR *entry = *entries;
			struct dirent *dent = readdir(entry);
			while (dent) {
				
				if (dent->d_type == DT_DIR && strcmp("..", dent->d_name) && strcmp(".", dent->d_name)) {
					/* For each entry, add it to the list if it doesn't exist in the hash table */
					if (!dictionary_get_int(dic, dent->d_name, &tmp)) {
						dictionary_set_int(dic, dent->d_name, tmp);
						
						snprintf(child_task.path, PATH_MAX-1, "%s/%s", task->path, dent->d_name);
						replication_queue_task(&child_task, OP_PRI_SYNC_FILE, 1);
					}
				}
				
				dent = readdir(entry);
			}
			closedir(entry); /* Don't forget to close the DIR after use! */
			entries++;
		}
		/* Free memory */
		dictionary_destroy(dic);
		free(entries_start);
	}

}

void replication_task_balance_file(ReplicationTask_t *task) {

	EXLog(REPL, DBG, "Task: Balance file [%s]", task->path);
	
	/* Sanity */
	if (!task->path) return;

	/* Indicate that this is the file we're replicating */
	pthread_mutex_lock(&s_replication_halt_replication_mutex);
	strncpy(s_replication_currently_replicating_file, task->path, PATH_MAX);
	pthread_mutex_unlock(&s_replication_halt_replication_mutex);
	
	/* Check if the file is open.  If so, we're not going to mess with it yet */
	if (get_open_fh_for_path(task->path, NULL, NULL)) {
		EXLog(REPL, DBG, " > File is open; skipping");
		return;
	}
	
	/* Update volume counters */
	volume_update_all_byte_counters();
	
	/* If the file has conflicts still, resolve them and break */
	mode_t mode = 0;
	int    mode_conflict = 0;
	int    instances;
	int    absences;
	
	RaidVolume_t *poss_volume_less_aff;
	RaidVolume_t *unpo_volume_most_aff;
	char fullpath_to_poss[PATH_MAX];
	char fullpath_to_unpo[PATH_MAX];
		
	volume_diagnose_raid_file_posession(task->path, &instances, &absences, &mode, &mode_conflict, &poss_volume_less_aff, fullpath_to_poss, &unpo_volume_most_aff, fullpath_to_unpo);
	if (mode_conflict) {
		volume_resolve_conflicting_modes(task->path);
		EXLog(REPL, DBG, " > File [%s] had a mode conflict", task->path);
		return;
	}
	
	/* If there are no absences, or we're at the proper instance level, we're done already */
	if (!absences || (instances == s_replication_min_redundancy)) {
		return;
	}
	
	/* If instances is more than our required redundancy, remove the file if the least-aff volume is above a certain threshold */
	if (instances > s_replication_min_redundancy) {
		if (poss_volume_less_aff->percent_free < s_replication_overredundant_perc_thresh) {
			unlink(fullpath_to_poss);
			EXLog(REPL, DBG, " > File [%s] was removed from [%s] - too many instances", task->path, poss_volume_less_aff->basepath);
			return;
		}
	}
	
	EXLog(REPL, DBG, "ED");
	
	/* Now we perform different duplications based on file type */
	switch (mode & S_IFMT) {
		case S_IFREG: {
			EXLog(REPL, DBG, "EE");
			if (__replication_copy_regular_file(task->path, unpo_volume_most_aff)) {
				/* If the copy succeeded, we should requeue it in case we need more */
				/* TODO: Maybe not, since we want to spread out duplications? */
			}
		} break;
	}
	
}

void replication_task_verify_raid_dir(ReplicationTask_t *task) {
	
}

/* ---------------------------- Replication Thread ------------------------------- */

void *replication_thread(void *arg) {
	
	EXLog(REPL, INFO, "Replication thread started");
	
	while (1) {
		
		ReplicationTask_t *next_task = (ReplicationTask_t*)tiered_priority_queue_pop(s_replication_queue);
				
		if (next_task) {
			
			switch (next_task->opcode) {
				case REP_OP_MIRROR_DIRECTORY:               replication_task_mirror_directory(next_task);                     break;
				case REP_OP_BALANCE_FILE:                   replication_task_balance_file(next_task);                         break;
				case REP_OP_BALANCE_FILES_IN_DIR:           replication_task_balance_files_in_dir(next_task);                 break;
				case REP_OP_VERIFY_VOLUME_DIRS:             replication_task_verify_raid_dir(next_task);                      break;
			}
			
			/* Free the task structure */
			free(next_task);
			
			/* Let other threads run */
			usleep(0);
			
		} else {
	
			EXLog(REPL, INFO, "Replication thread idle");
			
			/* Sleep a second before checking for a non-empty task stack */
			sleep(1);
		}		
	}
	
	pthread_exit(NULL);
}

