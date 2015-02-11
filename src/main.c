
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <git2.h>

#define MAX_DIRECTORY_ENTRIES 100
#define MAX_PATH_LENGTH 256

typedef enum { false, true } bool;

void display_list(char **list, int size) {
  int index;
  for (index = 0; index < size; index++)
    printf("%s\n", list[index]);
}

int subdirectories(char* dirname, char** subdirs) {
  struct dirent *dirent;
  int subdirs_count = 0;
  int index;

  DIR *directory = opendir(dirname);

  if(directory) {
    index = 0;

    while ((dirent = readdir(directory)) != NULL) {
      struct stat st;

      if(strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0)
        continue;
      if (fstatat(dirfd(directory), dirent->d_name, &st, 0) < 0) {
        perror(dirent->d_name);
        continue;
      }

      if (S_ISDIR(st.st_mode))
        subdirs[index++] = dirent->d_name;
    }

    subdirs_count = index;
    closedir(directory);
  } else {
    printf("Directory %s can't be opened\n", dirname);
    return 0;
  }

  return subdirs_count;
}

int filter_git_repos(char* dirname, char** subdirs, int size,
char** repos_names) {
  int index;
  int git_repo_count = 0;

  // repos_names = malloc(sizeof(char*) * size);

  for (index = 0; index < size; index++) {
    char *full_path = malloc(sizeof(char) * MAX_PATH_LENGTH);
    strcat(full_path, dirname);
    strcat(full_path, subdirs[index]);
    strcat(full_path, "/");
    if (git_repository_open_ext(NULL, full_path,
    GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0) {
      repos_names[git_repo_count++] = full_path;
    }
  }
  return git_repo_count;
}

char* fix_dirpath(char *dirname) {
  if (dirname[strlen(dirname)-1] != '/') {
    char *fix_dirname = malloc(strlen(dirname)+2);
    strcat(fix_dirname, dirname);
    fix_dirname[strlen(dirname)] = '/';
    fix_dirname[strlen(dirname)+1] = '\0';
    return fix_dirname;
  }
  return dirname;
}

int names_to_git_repos(char** names, int size, git_repository** git_repos) {
  int index;
  int repos_read_count = 0;

  for (index = 0; index < size; index++) {
    int error = git_repository_open(&git_repos[repos_read_count], names[index]);
    if (error < 0) {
      const git_error *e = giterr_last();
      printf("Error %d/%d: %s\n", error, e->klass, e->message);
    } else {
      repos_read_count++;
    }
  }

  return repos_read_count;
}

/*
checks for the presence of three things in a git repo -
    upstream remote
    dev branch
    master branch
*/
bool is_auto_updatable_repo(git_repository *repo) {
  int found_upstream = false;
  int found_master_dev = false;

  git_remote *out = NULL;
  git_reference *ref_out = NULL;

  if(git_remote_lookup(&out, repo, "upstream"))
    found_upstream = true;

  if(git_branch_lookup(&ref_out, repo, "master", GIT_BRANCH_REMOTE) &&
    git_branch_lookup(&ref_out, repo, "dev", GIT_BRANCH_REMOTE))
    found_master_dev = true;

  if(found_upstream && found_master_dev)
    return true;
  return false;
}

int filter_updateable_repos(git_repository **repos, int size,
git_repository **updatable_repos) {
  int index;
  int updateable_repos_count = 0;

  for (index = 0; index < size; index++) {
    if (is_auto_updatable_repo(repos[index]))
      updatable_repos[updateable_repos_count++] = repos[index];
  }

  return updateable_repos_count;
}

void update_repo(git_repository *repo) {
  git_strarray *remote_names = malloc(sizeof(git_strarray));
  git_remote *remote = NULL;
  int index;

  git_remote_list(remote_names, repo);

  for (index = 0; index < remote_names->count; index++) {
    git_remote_lookup(&remote, repo, remote_names->strings[index]);
    git_remote_connect(remote, GIT_DIRECTION_PUSH);
  }
}

int main(int argc, char **argv) {
  char *dirname = argv[1];
  char **subdirs_names = malloc(sizeof(char*) * MAX_DIRECTORY_ENTRIES);
  char **repos_names = NULL;
  git_repository **repos = NULL;
  git_repository **updateable_repos = NULL;

  int subdirs_count;
  int repos_count;
  int success_read_repos_count;
  int updateable_repos_count;

  git_libgit2_init();

  dirname = fix_dirpath(dirname);
  subdirs_count = subdirectories(dirname, subdirs_names);

  // TODO - Move such memory allocations inside the called function
  // using another level of pointer
  repos_names = malloc(sizeof(char*) * subdirs_count);
  repos_count = filter_git_repos(dirname, subdirs_names,
    subdirs_count, repos_names);

  repos = malloc(sizeof(git_repository*) * repos_count);
  success_read_repos_count = names_to_git_repos(repos_names,
    repos_count, repos);

  updateable_repos = malloc(sizeof(git_repository*) * repos_count);
  updateable_repos_count = filter_updateable_repos(repos,
    repos_count, updateable_repos);

  printf("Identified %d repos for auto sync.\n", updateable_repos_count);
  // display_list(repos_names, repos_count);

  while (1) {
    for (int index = 0; index < repos_count; index++) {
      update_repo(repos[index]);
    }
    sleep(3600);
  }

  git_libgit2_shutdown();

  return 0;
}
