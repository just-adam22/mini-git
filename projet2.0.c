//pour la Compilation : gcc -o projet2.0 projet2.1.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

//les STRUCTURES

typedef struct cell {
    char* data;
    struct cell* next;
} Cell;

typedef Cell* List;

typedef struct {
    char* name;
    char* hash;
    int   mode;
} WorkFile;

typedef struct {
    WorkFile* tab;
    int size;
    int n;
} WorkTree;

#define WORKTREE_SIZE 100


// CHAPITRE 1 — ENREGISTREMENT INSTANTANÉ

/* ---- Section 1.1 : Prise en main du langage Bash ---- */

// Q-1.2
//system("cat main.c | sha256sum > file.tmp");


// Q-1.3 : Calcule le hash SHA256 du fichier source et l'écrit dans dest
int hashFile(char* src, char* dest) {
    char cmd[1024];
    //from stdio.h 3andna snprintf bch n'affichiw les string fel format ly 7ajetna biha
    snprintf(cmd, sizeof(cmd), "sha256sum %s > %s", src, dest);
    return system(cmd);
}

// Q-1.4 : Renvoie le hash SHA256 du fichier donné sous forme de chaîne 
char* sha256file(char* file) {

    // nasnn3ou fichier temporaire
    static char tmpl[] = "/tmp/hashfileXXXXXX";
    char fname[1024];
    strcpy(fname, tmpl);

    //  el mkstemp cree le fichier temp o tafs5ou
    int fd = mkstemp(fname);
    if (fd == -1) 
        return NULL;
    close(fd);

    /* alternative lel commande : cat <file> | sha256sum > <tmp> */
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cat %s | sha256sum > %s", file, fname);
    system(cmd);

    /* na9raw el hash men west el fichier temporaire */
    FILE* f = fopen(fname, "r");
    if (!f) { unlink(fname); return NULL; }

    char* hash = malloc(65);
    if (fscanf(f, "%64s", hash) != 1) {
        free(hash);
        fclose(f);
        unlink(fname);
        return NULL;
    }
    fclose(f);
    unlink(fname); /* nafs5ou el fichier temporaire */
    return hash;
}

/* ---- Section 1.2 : Implémentation d'une liste chaînée de chaînes ---- */

//question2

/* Q-2.1 : Initialise une liste vide */
List* initList() {
    List* L = malloc(sizeof(List));
    *L = NULL;
    return L;
}

/* Q-2.2 : Alloue et retourne une cellule */
Cell* buildCell(char* ch) {
    Cell* c = malloc(sizeof(Cell));
    //strdup na3mlouha bech bch na3mlou duplication lel string
    c->data = strdup(ch);
    c->next = NULL;
    return c;
}

/* Q-2.3 : Ajoute un élément en tête de liste */
void insertFirst(List* L, Cell* C) {
    C->next = *L;
    *L = C;
}

/* Q-2.4a : Retourne la chaîne de caractères d'une cellule */
char* ctos(Cell* c) {
    return c->data;
}

/* Q-2.4b : Transforme une liste en chaîne "ch1|ch2|ch3|..." */
char* ltos(List* L) {
    // el l hyya pointeur mte3 awell element fel liste
    size_t total = 0;
    Cell* actuelle = *L;
    while (actuelle) {
        total += strlen(actuelle->data) + 1; // +1 pour '|' ou '\0'
        actuelle = actuelle->next;
    }
    char* s = malloc(total + 1);
    s[0] = '\0';
    actuelle = *L;
    while (actuelle) {
        //strcat ta3mel concatination 3ally fy west el result o el separation hyya he4y |
        strcat(s, actuelle->data);
        if (actuelle->next) strcat(s, "|");
        actuelle = actuelle->next;
    }
    return s;
}

/* Q-2.5 : Renvoie le i element d'une liste (base 0) */
Cell* listGet(List* L, int i) {
    Cell* current = *L;
    int idx = 0;
    while (current && idx < i) { 
        current = current->next; idx++; 
    }
    return current;
}

/* Q-2.6 : Recherche un element par contenu, retourne NULL si absent */
Cell* searchList(List* L, char* str) {
    Cell* cur = *L;
    while (cur) {
        //strcmp pour comparer 2 strings
        if (strcmp(cur->data, str) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Q-2.7 : Transforme une chaîne "ch1|ch2|ch3" en liste chainee */ //StringTolist
List* stol(char* s) {
    List* L = initList();
    if (!s || strlen(s) == 0) return L;
    char* copy = strdup(s); 
    char* token = strtok(copy, "|");//strtok nesta3mlouha bech n9ossou el string l'tokens
    /* On insère en tête puis on inverse pour préserver l'ordre */
    List* tmp = initList();
    while (token) {
        insertFirst(tmp, buildCell(token));
        token = strtok(NULL, "|");}
    /* Inversion pour retrouver l'ordre original */
    Cell* cur = *tmp;
    List* result = initList();
    /* Construire un tableau pour inverser */
    int count = 0;
    Cell* c = *tmp;
    while (c) { count++; c = c->next; }
    char** arr = malloc(count * sizeof(char*));
    c = *tmp;
    for (int i = 0; i < count; i++) { arr[i] = c->data; c = c->next; }
    for (int i = count - 1; i >= 0; i--)
        insertFirst(result, buildCell(arr[i]));
    free(arr);
    free(copy);
    free(tmp);
    return result;
}

/* Q-2.8a : Écrit une liste dans un fichier */
//listTofile
void ltof(List* L, char* path) {
    FILE* f = fopen(path, "w");
    char* s = ltos(L);
    fprintf(f, "%s", s);
    free(s);
    fclose(f);
}
/* Q-2.8b : Lit une liste depuis un fichier */
List* ftol(char* path) {
    FILE* f = fopen(path, "r");//fopen t7ell el path (chemin) ly 7ajetna bih
    fseek(f, 0, SEEK_END);//fseek tlawej 3la pos mou3ayyna o el SEEK_END tlawej 3al fin mta3 el liste
    long len = ftell(f);//ftell tjib el position mta3 el pointeur fy el file
    rewind(f);//rewind trajja3 awel position
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);//fread ta9ra el data mte3 file
    buf[len] = '\0';// el \0 ta3ny fin de chaine
    fclose(f);
    /* Retirer le '\n' final si présent */
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    List* L = stol(buf);
    free(buf);
    return L;
}

/* ---- Section 1.3 : Gestion de fichiers sous git ---- */

/* Q-3.1 : Renvoie une liste des fichiers/répertoires dans root_dir */
List* listdir(char* root_dir) {
    List* L = initList();
    
    DIR* dp = opendir(root_dir);//opendir t7ell el directory o readdir ta9rah
    if (!dp) return L;
    struct dirent* ep;
    while ((ep = readdir(dp)) != NULL) {
        /* Exclure "." et ".." */
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;
        insertFirst(L, buildCell(ep->d_name));
    }
    closedir(dp);
    return L;
}

/* Q-3.2 : Retourne 1 si le fichier existe dans le répertoire courant */
int file_exists(char* file) {
    List* L = listdir(".");
    Cell* found = searchList(L, file);
    return found != NULL ? 1 : 0;
}

/* Q-3.3 : Copie le contenu de 'from' vers 'to' ligne par ligne */
void cp(char* to, char* from) {
    if (!file_exists(from)) {
        fprintf(stderr, "Erreur : le fichier source '%s' n'existe pas.\n", from);
        return;
    }
    FILE* src = fopen(from, "r");
    FILE* dst = fopen(to, "w");
    if (!src || !dst) {
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }
    char line[4096];
    while (fgets(line, sizeof(line), src))
        fputs(line, dst);
    fclose(src);
    fclose(dst);
}

/* Q-3.4 : Retourne le chemin d'un fichier à partir de son hash
 *          (insère '/' entre le 2e et le 3e caractère) */
char* hashToPath(char* hash) {
    if (!hash || strlen(hash) < 3) return NULL;
    char* path = malloc(strlen(hash) + 2); /* +1 pour '/', +1 pour '\0' */
    path[0] = hash[0];
    path[1] = hash[1];
    path[2] = '/';
    strcpy(path + 3, hash + 2);
    return path;
}

/* Q-3.5 : Enregistre un instantané du fichier donné */
void blobFile(char* file) {
    char* hash = sha256file(file);
    if (!hash) return;

    char* path = hashToPath(hash);

    /* Créer le répertoire (les 2 premiers caractères du hash) */
    char dir[3];
    dir[0] = hash[0];
    dir[1] = hash[1];
    dir[2] = '\0';

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir);
    system(cmd);

    /* Copier le fichier vers son chemin de hash */
    cp(path, file);

    free(hash);
    free(path);
}

// CHAPITRE 2 — ENREGISTREMENT DE PLUSIEURS INSTANTANÉS

/* ---- Section 2.1 : Fonctions de manipulation de base ---- */

/* nchoufou les autorisations mte3 el fichier */
int getChmod(const char* path) {
    struct stat ret;
    if (stat(path, &ret) == -1) return -1;
    return (ret.st_mode & S_IRUSR) | (ret.st_mode & S_IWUSR) | (ret.st_mode & S_IXUSR) |
           (ret.st_mode & S_IRGRP) | (ret.st_mode & S_IWGRP) | (ret.st_mode & S_IXGRP) |
           (ret.st_mode & S_IROTH) | (ret.st_mode & S_IWOTH) | (ret.st_mode & S_IXOTH);
}

/* nbaddloui l'autorisations d'un fichier */
void setMode(int mode, char* path) {
    char buff[100];
    sprintf(buff, "chmod %d %s", mode, path);
    system(buff);
}

// nthabbtou i4a koll path houwa directory
int isDirectory(const char* path) {
    struct stat st;
    if (stat(path, &st) == -1) return 0;
    return S_ISDIR(st.st_mode);
}

/* Q-4.1 : Crée et initialise un WorkFile */
WorkFile* createWorkFile(char* name) {
    WorkFile* wf = malloc(sizeof(WorkFile));
    wf->name = strdup(name);
    wf->hash = NULL;
    wf->mode = 0;
    return wf;
}

/* Q-4.2 : Convertit un WorkFile en chaîne "name\thash\tmode" */
char* wfts(WorkFile* wf) {
    char mode_str[32];
    snprintf(mode_str, sizeof(mode_str), "%d", wf->mode);
    const char* hash = wf->hash ? wf->hash : "NULL";

    size_t len = strlen(wf->name) + strlen(hash) + strlen(mode_str) + 3;
    char* s = malloc(len);
    snprintf(s, len, "%s\t%s\t%s", wf->name, hash, mode_str);
    return s;
}

/* Q-4.3 : Convertit une chaîne "name\thash\tmode" en WorkFile */
WorkFile* stwf(char* ch) {
    char* copy = strdup(ch);
    WorkFile* wf = malloc(sizeof(WorkFile));

    char* tok = strtok(copy, "\t");
    wf->name = tok ? strdup(tok) : strdup("");

    tok = strtok(NULL, "\t");
    if (tok && strcmp(tok, "NULL") != 0)
        wf->hash = strdup(tok);
    else
        wf->hash = NULL;

    tok = strtok(NULL, "\t");
    wf->mode = tok ? atoi(tok) : 0;

    free(copy);
    return wf;
}

/* Q-4.4 : Alloue et initialise un WorkTree de taille fixée */
WorkTree* initWorkTree() {
    WorkTree* wt = malloc(sizeof(WorkTree));
    wt->tab  = malloc(WORKTREE_SIZE * sizeof(WorkFile));
    wt->size = WORKTREE_SIZE;
    wt->n    = 0;
    return wt;
}

/* Q-4.5 : Vérifie la présence d'un fichier dans le WorkTree.
 *          Retourne sa position ou -1. */
int inWorkTree(WorkTree* wt, char* name) {
    for (int i = 0; i < wt->n; i++) {
        if (strcmp(wt->tab[i].name, name) == 0) return i;
    }
    return -1;
}

/* Q-4.6 : Ajoute un fichier au WorkTree s'il n'existe pas déjà */
int appendWorkTree(WorkTree* wt, char* name, char* hash, int mode) {
    if (inWorkTree(wt, name) != -1) return -1; /* deja mawjoud */
    if (wt->n >= wt->size) return -1;           /* tableau plein */
    wt->tab[wt->n].name = strdup(name);
    wt->tab[wt->n].hash = hash ? strdup(hash) : NULL;
    wt->tab[wt->n].mode = mode;
    wt->n++;
    return wt->n - 1;
}

/* Q-4.7 : Convertit un WorkTree en chaîne de WorkFile séparés par '\n' */
char* wtts(WorkTree* wt) {
    size_t total = 0;
    char** lines = malloc(wt->n * sizeof(char*));
    for (int i = 0; i < wt->n; i++) {
        lines[i] = wfts(&wt->tab[i]);
        total += strlen(lines[i]) + 1; /* +1 pour '\n' */
    }
    char* s = malloc(total + 1);
    s[0] = '\0';
    for (int i = 0; i < wt->n; i++) {
        strcat(s, lines[i]);
        if (i < wt->n - 1) strcat(s, "\n");
        free(lines[i]);
    }
    free(lines);
    return s;
}

/* Q-4.8 : Convertit une chaîne représentant un WorkTree en WorkTree */
WorkTree* stwt(char* ch) {
    WorkTree* wt = initWorkTree();
    if (!ch || strlen(ch) == 0) return wt;

    char* copy = strdup(ch);
    char* line = strtok(copy, "\n");
    while (line) {
        WorkFile* wf = stwf(line);
        if (wt->n < wt->size) {
            wt->tab[wt->n].name = wf->name;
            wt->tab[wt->n].hash = wf->hash;
            wt->tab[wt->n].mode = wf->mode;
            wt->n++;
            free(wf); /* libère la structure mais pas les champs (réutilisés) */
        } else {
            free(wf->name);
            free(wf->hash);
            free(wf);
        }
        line = strtok(NULL, "\n");
    }
    free(copy);
    return wt;
}

/* Q-4.9 : Écrit la représentation d'un WorkTree dans un fichier */
int wttf(WorkTree* wt, char* file) {
    FILE* f = fopen(file, "w");
    if (!f) return -1;
    char* s = wtts(wt);
    fprintf(f, "%s", s);
    free(s);
    fclose(f);
    return 0;
}

/* Q-4.10 : Construit un WorkTree depuis un fichier */
WorkTree* ftwt(char* file) {
    FILE* f = fopen(file, "r");
    if (!f) return initWorkTree();

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    WorkTree* wt = stwt(buf);
    free(buf);
    return wt;
}

/* ---- Section 2.2 : Enregistrement instantané et restauration ---- */

/* Q-5.1 : Crée un instantané du WorkTree (avec extension ".t")
 *          et retourne son hash */
char* blobWorkTree(WorkTree* wt) {
    /* Fichier temporaire pour représenter le WorkTree */
    static char tmpl[] = "/tmp/worktreeXXXXXX";
    char fname[1000];
    strcpy(fname, tmpl);
    int fd = mkstemp(fname);
    if (fd == -1) return NULL;
    close(fd);

    wttf(wt, fname);

    char* hash = sha256file(fname);
    unlink(fname);
    if (!hash) return NULL;

    char* path = hashToPath(hash);

    /* Créer le répertoire */
    char dir[3] = { hash[0], hash[1], '\0' };
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir);
    system(cmd);

    /* Écrire le WorkTree dans son chemin de hash avec extension ".t" */
    char dest[512];
    snprintf(dest, sizeof(dest), "%s.t", path);
    wttf(wt, dest);

    free(path);
    return hash;
}

/* Q-5.2 : Enregistre recursivement tout le contenu d'un WorkTree */
char* saveWorkTree(WorkTree* wt, char* path) {
    for (int i = 0; i < wt->n; i++) {
        WorkFile* wf = &wt->tab[i];

        /* Construire le chemin complet */
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, wf->name);

        if (isDirectory(fullpath)) {
            /* Répertoire : appel récursif */
            List* contents = listdir(fullpath);
            WorkTree* newWT = initWorkTree();

            Cell* cur = *contents;
            while (cur) {
                appendWorkTree(newWT, cur->data, NULL, 0);
                cur = cur->next;
            }

            char* subHash = saveWorkTree(newWT, fullpath);
            wf->hash = subHash;
            wf->mode = getChmod(fullpath);
        } else {
            /* Fichier : blobFile */
            blobFile(fullpath);
            wf->hash = sha256file(fullpath);
            wf->mode = getChmod(fullpath);
        }
    }

    return blobWorkTree(wt);
}

/* Q-5.3 : Restaure récursivement un WorkTree dans le répertoire path */
void restoreWorkTree(WorkTree* wt, char* path) {
    for (int i = 0; i < wt->n; i++) {
        WorkFile* wf = &wt->tab[i];
        if (!wf->hash) continue;

        char* blobPath = hashToPath(wf->hash);

        /* Vérifier si c'est un répertoire (extension ".t") */
        char blobPathT[512];
        snprintf(blobPathT, sizeof(blobPathT), "%s.t", blobPath);

        FILE* test = fopen(blobPathT, "r");
        if (test) {
            /* C'est un répertoire */
            fclose(test);
            WorkTree* subWT = ftwt(blobPathT);

            /* Créer le répertoire de destination */
            char newPath[1024];
            snprintf(newPath, sizeof(newPath), "%s/%s", path, wf->name);
            char cmd[1200];
            snprintf(cmd, sizeof(cmd), "mkdir -p %s", newPath);
            system(cmd);

            restoreWorkTree(subWT, newPath);
        } else {
            /* C'est un fichier */
            char dest[1024];
            snprintf(dest, sizeof(dest), "%s/%s", path, wf->name);
            cp(dest, blobPath);
            setMode(wf->mode, dest);
        }

        free(blobPath);
    }
}

/* ============================================================
 * PROGRAMME PRINCIPAL — DÉMONSTRATION
 * ============================================================ */

int main() {
    printf("=== Projet Gestion de Versions ===\n\n");

    /* Exemple : hash d'un fichier */
    /* Créer un fichier de test */
    FILE* f = fopen("main.c", "w");
    if (f) {
        fprintf(f, "int main(){\n    return 0;\n}\n");
        fclose(f);
    }

    char* h = sha256file("main.c");
    if (h) {
        printf("Hash de main.c : %s\n", h);
        char* p = hashToPath(h);
        printf("Chemin de stockage : %s\n", p);
        free(p);
        free(h);
    }

    /* Exemple : WorkTree */
    WorkTree* wt = initWorkTree();
    appendWorkTree(wt, "main.c", NULL, 0);
    char* s = wtts(wt);
    printf("\nReprésentation WorkTree :\n%s\n", s);
    free(s);

    return 0;
}