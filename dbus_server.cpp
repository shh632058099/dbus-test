#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <dbus/dbus.h>
#include <chrono>

const char *const INTERFACE_NAME = "in.softprayog.dbus_example";
const char *const SERVER_BUS_NAME = "in.softprayog.add_server";
const char *const OBJECT_PATH_NAME = "/in/softprayog/adder";
const char *const METHOD_NAME = "test_dbus_damon";

DBusError dbus_error;
void print_dbus_error (char *str);
static DBusHandlerResult ws_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data);

uint32_t g_buffer_size = 0;

int main (int argc, char *argv[])
{
    g_buffer_size = atoi(argv[1]);

    DBusConnection *conn;
    int ret;

    dbus_error_init (&dbus_error);

    conn = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);

    if (dbus_error_is_set (&dbus_error))
        print_dbus_error ("dbus_bus_get");

    if (!conn) 
        exit (1);

    // Get a well known name
    ret = dbus_bus_request_name (conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);

    if (dbus_error_is_set (&dbus_error))
        print_dbus_error ("dbus_bus_get");

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf (stderr, "Dbus: not primary owner, ret = %d\n", ret);
        exit (1);
    }
#if 1
	dbus_error_init(&dbus_error);
    if (!dbus_connection_add_filter (conn, ws_dbus_message_handler, NULL, NULL)) { 
        fprintf(stderr, "dbus_connection_add_filter\n");
        exit(1); 
    } 
	while (dbus_connection_read_write_dispatch (conn, -1));
    return 0;

#endif

#if 0
    // Handle request from clients
    while (1) {
        // Block for msg from client
        if (!dbus_connection_read_write_dispatch (conn, -1)) {
            fprintf (stderr, "Not connected now.\n");
            exit (1);
        }
     
        DBusMessage *message;

        if ((message = dbus_connection_pop_message (conn)) == NULL) {
            fprintf (stderr, "Did not get message\n");
            continue;
        } 
        
        if (dbus_message_is_method_call (message, INTERFACE_NAME, METHOD_NAME)) {
            char *s;
            bool error = false;

            if (dbus_message_get_args (message, &dbus_error, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID)) {
                fprintf (stderr,"Get date from client: %s\n", s);
                
                if (!error) {
                    // send reply
                    DBusMessage *reply;
                    char answer [40];

                    sprintf (answer, "I have get your message!");
                    if ((reply = dbus_message_new_method_return (message)) == NULL) {
                        fprintf (stderr, "Error in dbus_message_new_method_return\n");
                        exit (1);
                    }
    
                    DBusMessageIter iter;
                    dbus_message_iter_init_append (reply, &iter);
                    char *ptr = answer;
                    if (!dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &ptr)) {
                        fprintf (stderr, "Error in dbus_message_iter_append_basic\n");
                        exit (1);
                    }

                    if (!dbus_connection_send (conn, reply, NULL)) {
                        fprintf (stderr, "Error in dbus_connection_send\n");
                        exit (1);
                    }

                    dbus_connection_flush (conn);
                
                    dbus_message_unref (reply);	
                }
                else // There was an error
                {
                    DBusMessage *dbus_error_msg;
                    char error_msg [] = "Error in input";
                    if ((dbus_error_msg = dbus_message_new_error (message, DBUS_ERROR_FAILED, error_msg)) == NULL) {
                         fprintf (stderr, "Error in dbus_message_new_error\n");
                         exit (1);
                    }

                    if (!dbus_connection_send (conn, dbus_error_msg, NULL)) {
                        fprintf (stderr, "Error in dbus_connection_send\n");
                        exit (1);
                    }

                    dbus_connection_flush (conn);
                
                    dbus_message_unref (dbus_error_msg);	
                }
            }
            else
            {
                print_dbus_error ("Error getting message");
            }
        }
    }
    return 0;
#endif
}

static DBusHandlerResult ws_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBusMessage *reply = NULL;
    const char * path =dbus_message_get_path(message);
    if(NULL==path){
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    typedef std::chrono::system_clock::duration system_time_test_t;
	if (strcmp(path,OBJECT_PATH_NAME) == 0) {
		if (dbus_message_is_method_call(message,INTERFACE_NAME,METHOD_NAME)) {

            // printf("ws_dbus_message_handler process\n");

			char *s;
            DBusMessageIter iter;
            DBusMessageIter sub_iter;
            dbus_message_iter_init(message, &iter);
       

            dbus_message_iter_recurse(&iter, &sub_iter);
            uint8_t* value;
			int nelements = 0;


			dbus_message_iter_get_fixed_array(&sub_iter, &value, &nelements);

            // printf("get fix arrary size:%d\n", nelements);
			if (1) {
                //get message from client
                system_time_test_t* tmpCnt = reinterpret_cast<system_time_test_t*>(value + g_buffer_size*1024);
                system_time_test_t count = *tmpCnt;
                const int calPerCnt = 10;

                

                static int i = 0;
                i++;
                static  system_time_test_t ticksum;
                std::chrono::system_clock::duration d = std::chrono::system_clock::now().time_since_epoch();
                // printf("get send timestamp:%ld send_recv timestamp:%ld\n",std::chrono::duration_cast<std::chrono::nanoseconds>(count).count(), std::chrono::duration_cast<std::chrono::nanoseconds>(d - count).count());
                ticksum += (d - count);
                if (i >= calPerCnt)
                {
                    fprintf(stderr, "BIG_PAYLOAD_SIZE_TEST size: %dK  times:%d stamp: %ld\n", g_buffer_size, calPerCnt, std::chrono::duration_cast<std::chrono::nanoseconds>(ticksum).count() / calPerCnt);
                    i = 0;
                    ticksum = std::chrono::system_clock::duration();
                }

                 
                //fprintf (stderr,"Get date from client: %s\n", s);
			}
				
			char answer [40];
			sprintf (answer, "I have get your message!");
			if ((reply = dbus_message_new_method_return (message)) == NULL) {
				fprintf (stderr, "Error in dbus_message_new_method_return\n");
				exit (1);
			}

			DBusMessageIter iter1;
			dbus_message_iter_init_append (reply, &iter1);
			char *ptr = answer;
			if (!dbus_message_iter_append_basic (&iter1, DBUS_TYPE_STRING, &ptr)) {
				fprintf (stderr, "Error in dbus_message_iter_append_basic\n");
				exit (1);
			}
		}
	}
	if(reply){
	// send the reply && flush the connection
	if (!dbus_connection_send(connection, reply, NULL)) {
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
		}
		dbus_connection_flush(connection);
		// free the reply
		dbus_message_unref(reply);

		return DBUS_HANDLER_RESULT_HANDLED ;
	}else{
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}	
}

void print_dbus_error (char *str) 
{
    fprintf (stderr, "%s: %s\n", str, dbus_error.message);
    dbus_error_free (&dbus_error);
}