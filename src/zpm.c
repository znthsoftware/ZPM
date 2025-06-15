#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <tar.h>
#include <unistd.h>
#include <dirent.h>

#define PACKAGE_INFO "/var/lib/zpm/package-info"
#define REPO_LIST "/etc/zpm/sources.list"
#define TEMP_DIR "/tmp/zpm"

void create_temp_dir() {
    mkdir(TEMP_DIR, 0755);
}

void remove_temp_dir() {
    system("rm -rf " TEMP_DIR);
}

void install_package(const char *package_path) {
    char command[256];
    snprintf(command, sizeof(command), "tar -xzf %s -C " TEMP_DIR, package_path);
    system(command);

    // Leer el archivo control
    char control_path[256];
    snprintf(control_path, sizeof(control_path), "%s/DEBIAN/control", TEMP_DIR);
    FILE *control_file = fopen(control_path, "r");
    if (control_file == NULL) {
        perror("Error opening control file");
        return;
    }

    char line[256];
    char package_name[256] = "";
    char version[256] = "";
    char depends[256] = "";
    while (fgets(line, sizeof(line), control_file)) {
        if (sscanf(line, "Package: %s", package_name) == 1) {
            continue;
        }
        if (sscanf(line, "Version: %s", version) == 1) {
            continue;
        }
        if (sscanf(line, "Depends: %s", depends) == 1) {
            continue;
        }
    }
    fclose(control_file);

    // Instalar dependencias
    if (strlen(depends) > 0) {
        char *token = strtok(depends, " ");
        while (token != NULL) {
            install_package_from_repo(token);
            token = strtok(NULL, " ");
        }
    }

    // Instalar el paquete
    snprintf(command, sizeof(command), "cp -r %s/* /", TEMP_DIR);
    system(command);

    // Guardar la información del paquete
    FILE *package_info = fopen(PACKAGE_INFO, "a");
    if (package_info == NULL) {
        perror("Error opening package info file");
        return;
    }
    fprintf(package_info, "%s %s\n", package_name, version);
    fclose(package_info);

    // Ejecutar postinst
    snprintf(command, sizeof(command), "%s/DEBIAN/postinst", TEMP_DIR);
    system(command);

    remove_temp_dir();
}

void install_package_from_repo(const char *package_name) {
    FILE *repo_list = fopen(REPO_LIST, "r");
    if (repo_list == NULL) {
        perror("Error opening repo list file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), repo_list)) {
        char repo_url[256];
        sscanf(line, "deb %s", repo_url);
        char package_path[256];
        snprintf(package_path, sizeof(package_path), "%s/%s.setup", repo_url, package_name);
        install_package(package_path);
    }
    fclose(repo_list);
}

void update_package_list() {
    FILE *repo_list = fopen(REPO_LIST, "r");
    if (repo_list == NULL) {
        perror("Error opening repo list file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), repo_list)) {
        char repo_url[256];
        sscanf(line, "deb %s", repo_url);
        char command[256];
        snprintf(command, sizeof(command), "wget -O - %s | tar -xzf - -C /", repo_url);
        system(command);
    }
    fclose(repo_list);
}

void remove_package(const char *package_name) {
    FILE *package_info = fopen(PACKAGE_INFO, "r");
    if (package_info == NULL) {
        perror("Error opening package info file");
        return;
    }

    char line[256];
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/tmp/package-info.tmp");
    FILE *temp_file = fopen(temp_path, "w");
    if (temp_file == NULL) {
        perror("Error opening temp file");
        fclose(package_info);
        return;
    }

    int found = 0;
    while (fgets(line, sizeof(line), package_info)) {
        if (strstr(line, package_name) != NULL) {
            found = 1;
            continue;
        }
        fputs(line, temp_file);
    }
    fclose(package_info);
    fclose(temp_file);

    if (found) {
        system("mv /tmp/package-info.tmp /var/lib/zpm/package-info");
        char command[256];
        snprintf(command, sizeof(command), "rm -rf /usr/share/%s", package_name);
        system(command);
        snprintf(command, sizeof(command), "/usr/share/%s/prerm", package_name);
        system(command);
    } else {
        printf("Paquete %s no encontrado.\n", package_name);
    }
}

void search_package(const char *package_name) {
    FILE *package_info = fopen(PACKAGE_INFO, "r");
    if (package_info == NULL) {
        perror("Error opening package info file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), package_info)) {
        if (strstr(line, package_name) != NULL) {
            printf("%s\n", line);
        }
    }
    fclose(package_info);
}

void list_installed_packages() {
    FILE *package_info = fopen(PACKAGE_INFO, "r");
    if (package_info == NULL) {
        perror("Error opening package info file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), package_info)) {
        printf("%s\n", line);
    }
    fclose(package_info);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: zpm <command> [arguments]\n");
        return 1;
    }

    create_temp_dir();

    if (strcmp(argv[1], "install") == 0 && argc == 3) {
        install_package(argv[2]);
    } else if (strcmp(argv[1], "update") == 0) {
        update_package_list();
    } else if (strcmp(argv[1], "remove") == 0 && argc == 3) {
        remove_package(argv[2]);
    } else if (strcmp(argv[1], "search") == 0 && argc == 3) {
        search_package(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        list_installed_packages();
    } else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }

    remove_temp_dir();
    return 0;
}