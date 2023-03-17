#include <memory>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

namespace net = boost::asio;
namespace sys = boost::system;
using std::chrono::steady_clock;

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
    using Strand = net::strand<net::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    Ticker(Strand& strand, std::chrono::milliseconds period, Handler handler);
        

    void Start();
private:
    void ScheduleTick();
    void OnTick(sys::error_code ec);

    Strand& strand_;
    net::steady_timer timer_;
    std::chrono::milliseconds period_;
    Handler handler_;
    std::chrono::milliseconds last_tick_;
};