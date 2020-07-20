#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <dbus/dbus.h>
#include "chrono"

const char *const INTERFACE_NAME = "in.softprayog.dbus_example";
const char *const SERVER_BUS_NAME = "in.softprayog.add_server";
const char *const CLIENT_BUS_NAME = "in.softprayog.add_client";
const char *const OBJECT_PATH_NAME = "/in/softprayog/adder";
const char *const METHOD_NAME = "test_dbus_damon";

DBusError dbus_error;
void print_dbus_error (char *str);
bool isinteger (char *ptr);

void print_dbus_error (char *str) 
{
    fprintf (stderr, "%s: %s\n", str, dbus_error.message);
    dbus_error_free (&dbus_error);
}

int main(int argc, char* argv[])
{
	DBusConnection *conn = NULL;
	DBusMessage     *msg = NULL;
	//DBusMessageIter args;
	DBusPendingCall  *pending = NULL;
    int ret;
    
	dbus_error_init(&dbus_error);
	conn = dbus_bus_get( DBUS_BUS_SYSTEM, &dbus_error);
	if( dbus_error_is_set( &dbus_error ) ){
		print_dbus_error ("dbus_bus_get");
		exit(1);
	}
	if(NULL == conn){
		exit(1);
	}
	// Get a well known name
#if 0
	/****这种方法有可能会停在此处**********/
	while (1) {
		ret = dbus_bus_request_name (conn, CLIENT_BUS_NAME, 0, &dbus_error);
        if (ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) 
        	break;
		if (ret == DBUS_REQUEST_NAME_REPLY_IN_QUEUE) {
			fprintf (stderr, "Waiting for the bus ... \n");
			sleep (1);
			continue;
		}
		if (dbus_error_is_set (&dbus_error))
			print_dbus_error ("dbus_bus_get");    
    }
	/*************************************************/	
#endif
#if 1	
 	ret = dbus_bus_request_name(conn, CLIENT_BUS_NAME,DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);
    if (dbus_error_is_set (&dbus_error))
        print_dbus_error ("dbus_bus_get");

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf (stderr, "Dbus: not primary owner, ret = %d\n", ret);
      	exit(1);
    }
#endif	

        //the first param is buffer size
    uint32_t buffer_size_raw = atoi(argv[1]);
    typedef std::chrono::system_clock::duration system_time_test_t;
    typedef uint8_t ByteType;

    std::uint32_t buffer_size = (buffer_size_raw * 1024 + sizeof(system_time_test_t));

    //ByteType buffer[buffer_size];
    // char buffer[buffer_size];
    // memset(buffer, 0xff, buffer_size);

    while (true) {

        //创建函数调用
        msg = dbus_message_new_method_call(SERVER_BUS_NAME, OBJECT_PATH_NAME, INTERFACE_NAME, METHOD_NAME);

        if(msg == NULL){
            fprintf (stderr, "Error in dbus_message_new_method_call\n");
            exit(1);
        }
        //开始添加发送数据
        /******
        dbus_message_iter_init_append(msg, &args);
        dbus_message_append_args(msg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &mac, 6, //传递数组
                                                        DBUS_TYPE_INT32, &port, DBUS_TYPE_INVALID);
        if (!dbus_message_iter_append_args (msg, DBUS_TYPE_STRING, &ptr,DBUS_TYPE_INVALID)) {
                dbus_message_unref(msg);//free msg
                fprintf (stderr, "Error in dbus_message_iter_append_args\n");
                exit(1);
        }
        ****/
        DBusMessageIter iter;
        DBusMessageIter array_iter;
        dbus_message_iter_init_append(msg, &iter);


        char tmp[2];
        tmp[0] = DBUS_TYPE_BYTE;
        tmp[1] = '\0';
        dbus_message_iter_open_container(&iter,
                                        DBUS_TYPE_ARRAY, tmp ,
                                        &array_iter);

        ByteType* array = new ByteType[buffer_size];
        const ByteType *v_ARRAY = array;


        //new buffer first and then assign
        system_time_test_t d = std::chrono::system_clock::now().time_since_epoch();
        memcpy((ByteType*)array + buffer_size_raw * 1024, &d, sizeof(d));
        //array[0] = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
        // dbus_int64_t buffer[] = {1};
        // const dbus_int64_t* v_ARRAY = buffer;
        


        dbus_message_iter_append_fixed_array (&array_iter, DBUS_TYPE_BYTE, &v_ARRAY, buffer_size);
        // dbus_message_iter_append_fixed_array(&array_iter, DBUS_TYPE_INT64, v_ARRAY, buffer_size);


        dbus_message_iter_close_container(&iter, &array_iter);

        if (!dbus_connection_send_with_reply(conn, msg, &pending, -1))
        {
            dbus_message_unref(msg);
            fprintf (stderr,"dbus_connection_send_with_reply failed");
            exit(1);
        }

        if (NULL == pending)
        {
            dbus_message_unref(msg);
            fprintf (stderr,"pending  == NULL");
            exit(1);
        }
        //发送数据
        dbus_connection_flush(conn);
        dbus_message_unref(msg);
        dbus_pending_call_block(pending);

        //printf("send data timestamp:%ld, buffer_size:%d\n", std::chrono::duration_cast<std::chrono::nanoseconds>(d).count(), buffer_size);
        
        //接收返回数据
        DBusMessage *reply;
        reply= dbus_pending_call_steal_reply(pending);
        if (reply== NULL)
        {
            fprintf (stderr,"call steal reply failed\n");
            dbus_message_unref(reply);
            exit(1);;
        }
        dbus_pending_call_unref(pending);
        char *s;
        //从服务器端返回一个简单的字符串
        if (dbus_message_get_args (reply, &dbus_error, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID)) {
            //printf ("Receive from server:  %s\n", s);
        }else{
            fprintf (stderr, "Did not get arguments in reply\n");
            exit (1);
        }
        dbus_message_unref (reply);	

        printf("buffer_size:%d Kb\n", buffer_size_raw);
        usleep(100*1000);
        delete []array;
    }
    

	/****************逐个解析获取信息***********
	dbus_error_init(&err);
	dbus_message_iter_init(msg, &iter);//成功返回1	
	if(DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&iter)){
		dbus_message_iter_get_basic(&iter, &status);//获取DBUS_TYPE_INT32类型数据存放到status
	}
	dbus_message_iter_next(&iter);//获取下一数据
	if(DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&iter)){
		dbus_message_iter_get_basic(&iter, &count);
	}
	//解析获取的数组数据
	DBusMessageIter sub_iter;
	if(dbus_message_iter_next(&iter){
		if(DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&iter)){
			dbus_message_iter_recurse(&iter, &sub_iter);
			dbus_message_iter_get_fixed_array(&sub_iter, &mac, &mac_len);
		}
	}
	*******************************************/

   	if (dbus_bus_release_name (conn, CLIENT_BUS_NAME, &dbus_error) == -1) {
        fprintf (stderr, "Error in dbus_bus_release_name\n");
        exit (1);
    }
	return 0;
}