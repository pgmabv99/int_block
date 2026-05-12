#ifndef WEBSOCKETPP_CONFIG_ASIO_TLS_CLIENT_HPP
#define WEBSOCKETPP_CONFIG_ASIO_TLS_CLIENT_HPP

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/transport/asio/security/tls.hpp>

namespace websocketpp
{
    namespace config
    {

        /// Asio client config with TLS support
        typedef asio_client base;

        struct asio_tls_client : public asio_client
        {
            typedef asio_tls_client type;
            typedef base::concurrency_type concurrency_type;
            typedef base::request_type request_type;
            typedef base::response_type response_type;
            typedef base::message_type message_type;
            typedef base::con_msg_manager_type con_msg_manager_type;
            typedef base::endpoint_msg_manager_type endpoint_msg_manager_type;

            typedef websocketpp::transport::asio::tls_socket::endpoint
                transport_config;

            typedef websocketpp::transport::asio::endpoint<transport_config>
                transport_type;

            static const long timeout_open_handshake = 30000;
        };

    } // namespace config
} // namespace websocketpp

#endif // WEBSOCKETPP_CONFIG_ASIO_TLS_CLIENT_HPP
