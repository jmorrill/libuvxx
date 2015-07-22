#include <iostream>
#include "uvxx.hpp"
#include <memory>

using namespace std;
using namespace uvxx;
using namespace uvxx::pplx;

int counter = 0;
uint64_t total_bytes_read = 0;

string host_name = "192.168.1.129";
int host_port = 80;

string http_command = "GET /movie.mp4 HTTP/1.0\r\n\r\n";

void test_method()
{
    struct socket_file_holder
    {
        socket_file_holder() : io_buff(io::memory_buffer(1024 * 1024)){}
        fs::file file;
        net::stream_socket socket;
        io::memory_buffer io_buff;
    };

    fs::file file;

    /* socket client */
    net::stream_socket socket;


    auto holder = make_shared<socket_file_holder>();

    holder->file = file;

    holder->socket = socket;

    /* Resolve host */
    socket.connect_async(host_name, host_port).

    then([socket](task<void> connect_task)
    {
        /* could throw, which handler below will catch */
        connect_task.get();

        cout << "connected !" << endl;

        /* Send http command */
        return socket.write_async(http_command);
    }).

    then([file](task<void> socket_write_task)
    {
        socket_write_task.get();

        return file.open_async("test.bin", std::ios_base::out);
    }).

    then([holder](task<void> t)
    {
        return create_iterative_task([holder]()
        {
            return holder->socket.read_async(holder->io_buff, 0, holder->io_buff.length_get()).

            then([holder](int bytes_read)
            {
                counter++;

                if (counter % 3000 == 0)
                {
                    printf("bytes read: %llu\n", total_bytes_read);
                }

                total_bytes_read += bytes_read;
              
                return holder->file.write_async(holder->io_buff, 0, bytes_read);
            });

        }, task_continuation_context::use_current());
    }).

    then([socket](task<void> iterative_task)
    {
        cout << "finished.." << endl;

        try
        {
            iterative_task.get();
        }
        catch (end_of_file const& ex)
        {
            cout << ex.what() << endl;
        }
        catch (exception const& ex)
        {
            cout << ex.what() << endl;
            throw;
        }

        return socket.shutdown_async();
    }).

    then([socket](task<void> shutdown_task)
    {
        printf("done\n");

        try
        {
            shutdown_task.get();
        }
        catch (exception const& ex)
        {
            cout << "general exception : " << ex.what() << endl;
        }

        event_dispatcher::current_dispatcher().begin_shutdown();
    });
}

int main(int argc, char** argv)
{
    auto dispatcher = event_dispatcher::current_dispatcher();
    cout << "thread id - " << this_thread::get_id() << endl;

    create_task([]
    {
        cout << "thread id 1 - " << this_thread::get_id() << endl;
    }, task_continuation_context::use_current()).
    then([]
    {
        cout << "thread id 2 - " << this_thread::get_id() << endl;
    }, task_continuation_context::use_current()).
    then([]
    {
        cout << "thread id 3 - " << this_thread::get_id() << endl;
    }, task_continuation_context::use_current());
   
    //test_method();

    struct ostream_holder
    {
        uvxx::streams::container_buffer<std::string> buffer;
        uvxx::streams::basic_ostream<uint8_t> output_stream;
    };

    auto oholder = std::make_shared<ostream_holder>();

   /* fs::file_stream<uint8_t>::open_ostream("output.txt").
    then([=](task<streams::basic_ostream<uint8_t>> t)
    {
        try
        {
            oholder->output_stream = t.get();
        }
        catch (exception const& e)
        {
        	cout << e.what() << endl;
        }
        oholder->output_stream.seek(10);
        return oholder->output_stream.print_line("wow");
    }).then([=](task<size_t> t)
    {
        auto size = t.get();

        return oholder->output_stream.flush();
    });*/

    event_dispatcher::run();

    cout << "total bytes read " << total_bytes_read << endl;
    cout << "exiting..." << endl;

    return 0;
}
