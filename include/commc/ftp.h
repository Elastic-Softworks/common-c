/* 	
   ===================================
   F T P . H
   FTP CLIENT IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    file transfer protocol (FTP) client implementation
	    providing comprehensive file transfer capabilities.
	    supports both active and passive modes with robust
	    error handling and educational clarity.
	       
	    implements standard FTP commands for file upload,
	    download, directory listing, and remote file
	    management operations with C89 compliance.

*/

#ifndef COMMC_FTP_H
#define COMMC_FTP_H

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
           --- CONSTANTS ---
	==================================
*/

#define COMMC_FTP_MAX_HOSTNAME_LENGTH   256
#define COMMC_FTP_MAX_USERNAME_LENGTH   64
#define COMMC_FTP_MAX_PASSWORD_LENGTH   64
#define COMMC_FTP_MAX_PATH_LENGTH       1024
#define COMMC_FTP_MAX_COMMAND_LENGTH    512
#define COMMC_FTP_MAX_RESPONSE_LENGTH   2048
#define COMMC_FTP_DEFAULT_PORT          21
#define COMMC_FTP_DEFAULT_TIMEOUT       30
#define COMMC_FTP_BUFFER_SIZE          8192

/* 
	==================================
            --- ENUMS ---
	==================================
*/

/*

         commc_ftp_mode_t
	       ---
	       enumeration of FTP transfer modes
	       for data connection establishment.

*/

typedef enum {
    COMMC_FTP_MODE_ACTIVE = 0,
    COMMC_FTP_MODE_PASSIVE
} commc_ftp_mode_t;

/*

         commc_ftp_type_t
	       ---
	       enumeration of FTP transfer types
	       for data representation.

*/

typedef enum {
    COMMC_FTP_TYPE_ASCII = 0,
    COMMC_FTP_TYPE_BINARY
} commc_ftp_type_t;

/*

         commc_ftp_state_t
	       ---
	       enumeration of FTP client connection
	       states for status tracking.

*/

typedef enum {
    COMMC_FTP_STATE_DISCONNECTED = 0,
    COMMC_FTP_STATE_CONNECTING,
    COMMC_FTP_STATE_CONNECTED,
    COMMC_FTP_STATE_AUTHENTICATED,
    COMMC_FTP_STATE_TRANSFERRING,
    COMMC_FTP_STATE_ERROR
} commc_ftp_state_t;

/* 
	==================================
           --- STRUCTURES ---
	==================================
*/

/*

         commc_ftp_response_t
	       ---
	       structure representing FTP server
	       response with code and message.

*/

typedef struct {
    int  code;                                    /* Response code (e.g., 200, 550) */
    char message[COMMC_FTP_MAX_RESPONSE_LENGTH]; /* Response message text */
    int  is_multiline;                           /* Multi-line response flag */
} commc_ftp_response_t;

/*

         commc_ftp_file_info_t
	       ---
	       structure containing file information
	       from directory listings.

*/

typedef struct {
    char name[COMMC_FTP_MAX_PATH_LENGTH];  /* File/directory name */
    long size;                             /* File size in bytes */
    int  is_directory;                     /* Directory flag */
    char permissions[16];                  /* Permission string */
    char date_modified[32];                /* Modification date */
    char owner[64];                        /* File owner */
    char group[64];                        /* File group */
} commc_ftp_file_info_t;

/*

         commc_ftp_client_t
	       ---
	       main FTP client structure containing
	       connection and configuration data.

*/

typedef struct {
    int                     control_socket;    /* Control connection socket */
    int                     data_socket;       /* Data connection socket */
    char                    hostname[COMMC_FTP_MAX_HOSTNAME_LENGTH];
    int                     port;              /* Server port */
    char                    username[COMMC_FTP_MAX_USERNAME_LENGTH];
    char                    password[COMMC_FTP_MAX_PASSWORD_LENGTH];
    commc_ftp_mode_t        mode;             /* Active/Passive mode */
    commc_ftp_type_t        type;             /* ASCII/Binary type */
    commc_ftp_state_t       state;            /* Connection state */
    int                     timeout;          /* Operation timeout */
    commc_ftp_response_t    last_response;    /* Last server response */
    char                    current_dir[COMMC_FTP_MAX_PATH_LENGTH];
} commc_ftp_client_t;

/* 
	==================================
        --- CLIENT MANAGEMENT ---
	==================================
*/

/*

         commc_ftp_client_create()
	       ---
	       creates and initializes a new FTP client
	       with default settings.

*/

commc_error_t commc_ftp_client_create(commc_ftp_client_t** client);

/*

         commc_ftp_client_destroy()
	       ---
	       destroys FTP client and cleans up
	       all associated resources.

*/

void commc_ftp_client_destroy(commc_ftp_client_t* client);

/*

         commc_ftp_set_timeout()
	       ---
	       configures timeout for FTP operations
	       in seconds.

*/

commc_error_t commc_ftp_set_timeout(commc_ftp_client_t* client,
                                    int                 timeout_seconds);

/*

         commc_ftp_set_mode()
	       ---
	       configures FTP transfer mode
	       (active or passive).

*/

commc_error_t commc_ftp_set_mode(commc_ftp_client_t* client,
                                 commc_ftp_mode_t    mode);

/*

         commc_ftp_set_type()
	       ---
	       configures FTP transfer type
	       (ASCII or binary).

*/

commc_error_t commc_ftp_set_type(commc_ftp_client_t* client,
                                 commc_ftp_type_t    type);

/* 
	==================================
        --- CONNECTION MANAGEMENT ---
	==================================
*/

/*

         commc_ftp_connect()
	       ---
	       establishes connection to FTP server
	       and performs initial handshake.

*/

commc_error_t commc_ftp_connect(commc_ftp_client_t* client,
                                const char*         hostname,
                                int                 port);

/*

         commc_ftp_authenticate()
	       ---
	       performs user authentication with
	       username and password.

*/

commc_error_t commc_ftp_authenticate(commc_ftp_client_t* client,
                                     const char*         username,
                                     const char*         password);

/*

         commc_ftp_disconnect()
	       ---
	       gracefully disconnects from FTP server
	       and cleans up connections.

*/

commc_error_t commc_ftp_disconnect(commc_ftp_client_t* client);

/* 
	==================================
        --- FILE OPERATIONS ---
	==================================
*/

/*

         commc_ftp_upload_file()
	       ---
	       uploads local file to remote server
	       with progress tracking capability.

*/

commc_error_t commc_ftp_upload_file(commc_ftp_client_t* client,
                                    const char*         local_path,
                                    const char*         remote_path);

/*

         commc_ftp_download_file()
	       ---
	       downloads remote file to local system
	       with progress tracking capability.

*/

commc_error_t commc_ftp_download_file(commc_ftp_client_t* client,
                                      const char*         remote_path,
                                      const char*         local_path);

/*

         commc_ftp_delete_file()
	       ---
	       deletes specified file from remote server
	       using DELE command.

*/

commc_error_t commc_ftp_delete_file(commc_ftp_client_t* client,
                                    const char*         remote_path);

/*

         commc_ftp_rename_file()
	       ---
	       renames file on remote server using
	       RNFR and RNTO commands.

*/

commc_error_t commc_ftp_rename_file(commc_ftp_client_t* client,
                                    const char*         old_name,
                                    const char*         new_name);

/*

         commc_ftp_get_file_size()
	       ---
	       retrieves file size from remote server
	       using SIZE command.

*/

commc_error_t commc_ftp_get_file_size(commc_ftp_client_t* client,
                                      const char*         remote_path,
                                      long*               file_size);

/* 
	==================================
        --- DIRECTORY OPERATIONS ---
	==================================
*/

/*

         commc_ftp_change_directory()
	       ---
	       changes current working directory
	       on remote server using CWD command.

*/

commc_error_t commc_ftp_change_directory(commc_ftp_client_t* client,
                                         const char*         directory);

/*

         commc_ftp_get_current_directory()
	       ---
	       retrieves current working directory
	       from remote server using PWD command.

*/

commc_error_t commc_ftp_get_current_directory(commc_ftp_client_t* client,
                                              char*               directory,
                                              size_t              buffer_size);

/*

         commc_ftp_create_directory()
	       ---
	       creates new directory on remote server
	       using MKD command.

*/

commc_error_t commc_ftp_create_directory(commc_ftp_client_t* client,
                                         const char*         directory);

/*

         commc_ftp_remove_directory()
	       ---
	       removes directory from remote server
	       using RMD command.

*/

commc_error_t commc_ftp_remove_directory(commc_ftp_client_t* client,
                                         const char*         directory);

/*

         commc_ftp_list_directory()
	       ---
	       retrieves directory listing from
	       remote server using LIST command.

*/

commc_error_t commc_ftp_list_directory(commc_ftp_client_t*    client,
                                       const char*            directory,
                                       commc_ftp_file_info_t* files,
                                       int                    max_files,
                                       int*                   file_count);

/* 
	==================================
        --- UTILITY FUNCTIONS ---
	==================================
*/

/*

         commc_ftp_send_command()
	       ---
	       sends raw FTP command to server
	       and retrieves response.

*/

commc_error_t commc_ftp_send_command(commc_ftp_client_t*    client,
                                     const char*            command,
                                     commc_ftp_response_t*  response);

/*

         commc_ftp_get_last_response()
	       ---
	       retrieves the last server response
	       for debugging and error analysis.

*/

commc_error_t commc_ftp_get_last_response(const commc_ftp_client_t* client,
                                          commc_ftp_response_t*     response);

/*

         commc_ftp_is_connected()
	       ---
	       checks if FTP client is connected
	       and authenticated with server.

*/

int commc_ftp_is_connected(const commc_ftp_client_t* client);

/*

         commc_ftp_get_state()
	       ---
	       retrieves current FTP client state
	       for status monitoring.

*/

commc_ftp_state_t commc_ftp_get_state(const commc_ftp_client_t* client);

/*

         commc_ftp_parse_file_list()
	       ---
	       parses directory listing response
	       into structured file information.

*/

commc_error_t commc_ftp_parse_file_list(const char*            list_data,
                                        commc_ftp_file_info_t* files,
                                        int                    max_files,
                                        int*                   file_count);

/* 
	==================================
        --- CONVENIENCE FUNCTIONS ---
	==================================
*/

/*

         commc_ftp_quick_upload()
	       ---
	       convenience function for simple
	       file upload with authentication.

*/

commc_error_t commc_ftp_quick_upload(const char* hostname,
                                     int         port,
                                     const char* username,
                                     const char* password,
                                     const char* local_path,
                                     const char* remote_path);

/*

         commc_ftp_quick_download()
	       ---
	       convenience function for simple
	       file download with authentication.

*/

commc_error_t commc_ftp_quick_download(const char* hostname,
                                       int         port,
                                       const char* username,
                                       const char* password,
                                       const char* remote_path,
                                       const char* local_path);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_FTP_H */

/* 
	==================================
             --- EOF ---
	==================================
*/