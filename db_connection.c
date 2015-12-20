#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Create two constants
#define MAX_DATA 512
#define MAX_ROWS 100

/* We create a struct with some fields*/
struct Address{
    int id;
    int set;
    char name[MAX_DATA];
    char email[MAX_DATA];
};

/* we create a database struct with a address struct as a field*/
struct Database{
    struct Address rows[MAX_ROWS];
};

/* we create a connection struct with fields like file, we use the object FILE wich contains all the information
 * necessary to control the stream and add the struct Database */
struct Connection{
    FILE *file;
    struct Database *db;
}

/* if there exist some error (errno means the int of the error) we print the message if there is not a recognized
 * error then print the passed message and we exit the program with a bad return int*/
void die(const char *message)
{
    if(errno){
        perror(message);
    }else{
        printf("ERROR: %s\n", message);
    }
    exit(1);
}

// we just print the values of the passed struct
void Address_print(struct Address *addr)
{
    printf("%d %s %s\n",
            addr->id, addr->name, addr->email);
}

/* fread is a function that reads data from the given stream into the array pointed
 * the first parameter : is the array that ww wean to read, in this case is a pointer to ablock memory
 * the second parameter: this is the size in bytes of each element to be read
 * the third parameter: this is the numberof elements, each one with a size of size bytes
 * the fourth parameter: this is the pointer to a FILE object that specifies an input stream*/
void Database_load(struct Connection *conn)
{
    int rc = fread(conn->db, sizeof(struct Database), 1, conn->file);
    // if exist an error we print the error message
    if(rc != 1) die("Failed to load database");
}


struct Connection *Database_open(const char *filename, char mode)
{
    // we create a connection and give some space in memory with the size of a struct connection
    struct Connection *con = malloc(sizeof(struct Connection));
    // if there was an error making the connection print the message
    if(!conn) die("Memory error");

    // we assing some space in memory to the db
    conn->db = malloc(sizeof(struct Database));
    if(!conn->db) die("Memory error");

    // if a c was chosen means that we want to create a new register
    // we assign the file of the connection to a file, w means open a text file for writing, 
    // if we put w+ opens a text file for both reading and writing
    if(mode == 'c'){
        conn->file = fopen(filename, "w");
    }else{
        // if not we open with both reading and writing
        conn->file = fopen(filename, "r+");
        // if theres a valid file in the connection read them
        if(conn->file){
            Database_load(conn);
        }
    }

    // if there's no file then print the error message
    if(!conn->file) die("Failed to open the file");
    
    return conn;
}

void Database_close(struct Connection *conn)
{
    // if the connection exists
    if(conn){
        // if there's a file close it.
        if(conn->file) fclose(conn->file);
        // free the space in memory of the db
        if(conn->db) free(conn->db);
        // free the connection
        free(conn);
    }
}


void Database_write(struct Connection *conn)
{
    // sets the file position to the beginning of the file of the given stream
    rewind(conn->file);

    // writes data from the array pointed to to the given stream
    int rc = fwrite(conn->db, sizeof(struct Database), 1, conn->file);
    if(rc != 1) die("Failed to write database");

    // flushes the output buffer os a stream
    rc = fflush(conn->file);
    if(rc == -1) die("Cannot flush database.");
}

void Database_create(struct Connection *conn)
{
    int i = 0;
    // we make some registers, for every row we create an address struct and 
    // initialize it's id with i and his set to 0, we assign it to some row
    for(i = 0; i < MAX_ROWS; i++){
        // make a prototype to initialize it
        struct Address addr = {.id = i, .set = 0};
        // then just assign it
        conn->db->rows[i] = addr;
    }
}


void Database_set(struct Connection *conn, int id, const char *name, const char *email)
{
    // we get the address struct passed the id;
    struct Address *addr = &conn->db->rows[id];
    // if this already has a set show message
    if(addr->set) die("Already set, delete it first");

    // if it doesn't have a set put 1
    addr->set = 1;
    // WARNING: bug, read the "How To Break It" and fix this
    char *res = strncpy(addr->name, name, MAX_DATA);
    // demonstrate the strncpy bug
    if(!res) die("Name copy failed");

    // we copy the email we send to the address email, we need to specify the large amount of data size
    res = strncpy(addr->email, email, MAX_DATA);
    // if it doesn't work show a message
    if(!res) die("Email copy failed");
}

void Database_get(struct Connection *conn, int id)
{
    // we get the address given the id
    struct Address *addr = &conn->db->rows[id];
    // if it has a set print all the address
    if(addr->set){
        Address_print(addr);
    }else{
        die("ID is not set");
    }
}

void Database_delete(struct Connection *conn, int id)
{
    // we put the set to 0 of the address struct specified
    struct Address addr = {.id = id, .set = 0};
    conn->db->rows[id] = addr;
}

void Database_list(struct Connection *conn)
{
    int i = 0;
    // get the database
    struct Database *db = conn->db;
    for(i = 0; i < MAX_ROWS; i++){
        // create a temporary struct
        struct Address *cur = &db->rows[i];

        // if has a set print it
        if(cur->set){
            Address_print(cur);
        }
    }
}

int main(int argc, char *argv[])
{
    // if there are less arguments print the format required
    if(argc < 3) die("USAGE: ex17 <dbfile> <action> [action params]");

    // the filename will be the first
    char *filename = argv[1];
    // the first letter of the second argument will be the action
    char action = argv[2][0];
    // we create a connection with the filename and action specified
    struct Connection *conn = Database_open(filename, action);
    int id = 0;

    // if there are more arguments, we cast the third argument to int
    if(argc > 3) id = atoi(argv[3]);
    // if the id is greater than the MAX_ROWS show message
    if(id >= MAX_ROWS) die("There's not that many records.");

    switch(action){
        case 'c':
            Database_create(conn);
            Database_write(conn);
            break;

        case 'g':
            if(argc != 4) die("Need an id to get");
            Database_get(conn, id);
            break;

        case 's':
            if(argc != 6) die("Need id, name, email to set");
            Database_set(conn, id, argv[4], argv[5]);
            Database_write(conn);
            break;

        case 'd':
            if(argc != 4) die("Need id to delete");
            Database_delete(conn, id);
            Database_write(conn);
            break;

        case 'l': 
            Database_list(conn);
            break;

        default:
            die("Invalid action, only: c=create, g=get, s=set, d=del, l=list");
    }
    Database_close(conn);
    return 0;
}
