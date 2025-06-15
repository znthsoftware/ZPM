// ZNTH SOFTAWRE COMPANY ALL RIGHTS RESERVED
#ifndef ZPM_H
#define ZPM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <tar.h>
#include <unistd.h>
#include <dirent.h>

// Definiciones de constantes
#define PACKAGE_INFO "/var/lib/zpm/package-info"
#define REPO_LIST "/etc/zpm/sources.list"
#define TEMP_DIR "/tmp/zpm"

// Estructuras
typedef struct {
    char name[256];
    char version[256];
    char depends[256];
} PackageInfo;

// Declaraciones de funciones
void create_temp_dir();
void remove_temp_dir();
void install_package(const char *package_path);
void install_package_from_repo(const char *package_name);
void update_package_list();
void remove_package(const char *package_name);
void search_package(const char *package_name);
void list_installed_packages();
PackageInfo *get_package_info(const char *package_name);
void free_package_info(PackageInfo *info);
char **split_string(const char *str, const char *delim);
int string_array_length(char **array);
void print_package_info(PackageInfo *info);

#endif // ZPM_H
