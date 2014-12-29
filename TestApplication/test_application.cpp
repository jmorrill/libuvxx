#include "stdafx.h"

using namespace std;
using namespace uvxx;
using namespace uvxx::pplx;

class application : public event_dispatcher_object
{
    event_dispatcher_timer _timer;

    void on_tick(event_dispatcher_timer* sender)
    {
        cout << "tick tock" << endl;

        dispatcher().begin_invoke([]
        {
            cout << "I'm on the dispatcher thread" << endl;
            return 69;
        }).then([](int value)
        {
            return 42.0;
        }).then([](double value)
        {
            cout << "I just got value " << value << " sent to me and I'm on the dispatcher loop thread" << endl;
        }, task_continuation_context::use_arbitrary()).then([this]()
        {
            cout << "I'm on a thread pool thread" << endl;
        });
    }
public:
    application()
    {
        _timer.repeat_set(chrono::milliseconds(1000));
        _timer.timeout_set(chrono::milliseconds(1000));
        
        /* Hook the tick event */
        _timer.tick_event() += std::bind(&application::on_tick, this, placeholders::_1);

        /* Hook tick event with lambda */
        _timer.tick_event() += [](event_dispatcher_timer* sender)
        {
            cout << "I'm the second handler of the tick event" << endl;
        };
    }

   /* application(const application& app) : event_dispatcher_object(app) = default;
    application& operator=(const application& app) = default;*/

    application(application&& app) : event_dispatcher_object(std::move(app))
    {
        _timer = move(app._timer);
    }

    application& operator=(application&& app) 
    { 
        _timer = move(app._timer);
        return static_cast<application&>(event_dispatcher_object::operator=(std::move(app))); 
    }

    void stop()
    {
        _timer.stop();
    }

    void start()
    {
        _timer.start();
    }
};

int read_pass_counter = 0;
uint64_t total_bytes_read = 0;

string host_name = "192.168.1.129";
int host_port = 80;

string http_command = "GET /movie.mp4 HTTP/1.0\r\n\r\n";

void test_method()
{
    /* convenience struct to hold data for async calls.  Also helps
    *  in decreasing atomic ops in create_iterative task*/
    struct socket_file_holder
    {
        socket_file_holder() : io_buff(io::memory_buffer(512 * 1024)){ }
        socket_file_holder(const socket_file_holder &) = delete;
        socket_file_holder& operator=(const socket_file_holder &) = delete;

        fs::file file;
        net::stream_socket socket;
        io::memory_buffer io_buff;
    };
    
    auto async_objects = make_shared<socket_file_holder>();

    async_objects->socket.connect_async(host_name, host_port).

    then([async_objects](task<void> connect_task)
    {
        connect_task.get();

        printf("connected to %s port %d\n", host_name.c_str(), host_port);

        printf("writing http GET cmd to socket\n", http_command.c_str());

        return async_objects->socket.write_async(http_command);
    }).

    then([async_objects](task<void> socket_write_task)
    {
        socket_write_task.get();

        printf("opening file test.bin to write http data to...\n");

        return async_objects->file.open_async("dump.bin", std::ios_base::out);
    }).

    then([async_objects]()
    {
        printf("begin downloading from socket\n");

        return create_iterative_task([async_objects]()
        {
            return async_objects->socket.read_async(async_objects->io_buff, 0, async_objects->io_buff.length_get()).

            then([async_objects](int bytes)
            {
                int bytes_read = 0;

                bytes_read = bytes;

                read_pass_counter++;

                if (read_pass_counter % 1000 == 0)
                {
                    printf("bytes read %llu\r\n", total_bytes_read);
                }

                total_bytes_read += bytes_read;

                return async_objects->file.write_async(async_objects->io_buff, 0, bytes_read);
            });

        }, task_continuation_context::use_current());
    }).

    then([async_objects](task<void> iterative_task)
    {
        cout << "finished downloading..." << endl;

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
        }

        return async_objects->socket.shutdown_async();
    }).
    then([async_objects](task<void> socket_shutdown_task)
    {
        try
        {
            socket_shutdown_task.get();
        }
        catch (exception const& ex)
        {
            cout << ex.what() << endl;
            throw;
        }

        return async_objects->file.close_async();
    }).
    then([async_objects](task<void> file_close_task)
    {
        try
        {
            file_close_task.get();
        }
        catch (exception const& ex)
        {
            printf("exception - %s\n", ex.what());
        }

        printf("done!\n");

        event_dispatcher::current_dispatcher().begin_shutdown();
    });
}

int main(int argc, _TCHAR* argv[])
{
    auto dispatcher = event_dispatcher::current_dispatcher();

    printf("starting libuvxx\n\n");
    cout << "current thread id " << this_thread::get_id() << endl;

    struct stream_holder
    {
        uvxx::streams::container_buffer<std::string> buffer;
        uvxx::streams::basic_istream<uint8_t> input_stream;
    };

    auto holder = std::make_shared<stream_holder>();
   
    fs::file_stream<uint8_t>::open_istream("C:\\Users\\Jeremiah\\Desktop\\lines.txt").
    then([=](task<streams::basic_istream<uint8_t>> t)
    {
        cout << "current thread id " << this_thread::get_id() << endl;

        holder->input_stream = t.get();

        holder->input_stream.seek(3);

        return holder->input_stream.read_line(holder->buffer);
    }).
    then([=](task<size_t> t)
    {
        try
        {
            auto size = t.get();
        }
        catch (std::exception const& e)
        {
            cout << e.what() << endl;
        }

        cout << holder->buffer.collection() << endl;

        holder->buffer = uvxx::streams::container_buffer<std::string>();
     holder->input_stream.seek(40);
        return holder->input_stream.read_line(holder->buffer);
    }).
    then([=](task<size_t> t)
    {
        try
        {
            auto size = t.get();
        }
        catch (std::exception const& e)
        {
            cout << e.what() << endl;
        }

        cout << holder->buffer.collection() << endl;
    });


    /*
        fs::file_buffer<uint8_t>::open("test1.bin").
        then([=](task<uvxx::streams::streambuf<uint8_t>> t)
        {
        try
        {
        auto b = t.get();
        holder->buff = b;
        return holder->buff.putn(mem_buffer, mem_buffer.length_get());
        }
        catch (std::exception& e)
        {
        cout << e.what() << endl;
        throw;
        }
        }).
        then([=](task<size_t> t)
        {
        t.get();

        return holder->buff.putn(mem_buffer, mem_buffer.length_get());
        });*/
   /*uvxx::fs::directory::create_directory_async("c:\\users\\jeremiah\\desktop\\abc\\def\\ghi\\jkl").
    then([](task<void> t)
    {
        printf("done making dir\n");
    });

    uvxx::fs::directory::get_files_async("C:\\Users\\Jeremiah\\", true).
    then([](task<uvxx::fs::directory_entry_result_ptr> t)
    {
        try
        {
           auto files = t.get();
           printf("found %d files\n", files->size());
        }
        catch (no_file_or_directory const& e)
        {
            printf("%s\n", e.what());
        }
        catch (std::exception&  e)
        {
             printf("%s\n", e.what());
        }
    });
    */
    //test_method();

    //fs::file::get_file_info_async("C:\\inetpub\\wwwroot\\movie.mp4").
    //then([](task<fs::file_info> info_task)
    //{
    //    auto info = info_task.get();
    //});;

    event_dispatcher::run();

    return 0;
}
