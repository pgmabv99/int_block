#pragma once

#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <curl/curl.h>
#include <ctime>
#include "json.hpp"
#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_client.hpp"

using json = nlohmann::json;
using TransactionCallback = std::function<void(const json &)>;

// Use the TLS version from asio_client.hpp
typedef websocketpp::client<websocketpp::config::asio_tls_client> WSClient;
typedef websocketpp::connection_hdl ConnectionHandle;

class BlockchainClient
{
private:
    std::string endpoint;
    std::string ws_endpoint;
    std::shared_ptr<WSClient> ws_client;
    ConnectionHandle ws_hdl;
    TransactionCallback tx_callback;

    // Callback for CURL to write response
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

    // Make JSON-RPC request
    json makeRequest(const std::string &method, const json &params);

    // WebSocket handlers
    void on_open(ConnectionHandle hdl);
    void on_close(ConnectionHandle hdl);
    void on_message(ConnectionHandle hdl, WSClient::message_ptr msg);
    void on_fail(ConnectionHandle hdl);

public:
    BlockchainClient(const std::string &infura_api_key);
    ~BlockchainClient();

    // Public stream management (caller manages creation/joining)
    bool streaming = false;
    std::thread stream_thread;
    int pending_transaction_count = 0;

    // Query functions
    json getBlockNumber();
    json getBlockByNumber(const std::string &blockNumber);
    json getBalance(const std::string &address);
    json getTransactionByHash(const std::string &txHash);
    json getGasPrice();
    json getTransactionCount(const std::string &address);

    // WebSocket streaming function (public, called via thread)
    void streamTransactionWorker(TransactionCallback callback);

    // Transaction handler method
    void onPendingTransaction(const json &tx);

    // Decode function selector from 4byte.directory
    std::string decodeFunctionSelector(const std::string &selector);
};

// Debug helper function - convert hex timestamp to readable format
inline std::string format_timestamp(const std::string &hex_timestamp)
{
    try
    {
        // Remove '0x' prefix if present
        std::string hex = hex_timestamp;
        if (hex.substr(0, 2) == "0x" || hex.substr(0, 2) == "0X")
        {
            hex = hex.substr(2);
        }

        // Convert hex to decimal (Unix timestamp)
        uint64_t timestamp = std::stoull(hex, nullptr, 16);

        // Convert to time_t for formatting
        time_t time = static_cast<time_t>(timestamp);
        struct tm *timeinfo = localtime(&time);

        // Format as human-readable string
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        return std::string(buffer);
    }
    catch (...)
    {
        return hex_timestamp;
    }
}

// Debug helper function - extract and display timestamp from JSON if present (top-level or nested in result)
inline void display_timestamp_if_present(const json &j)
{
    try
    {
        std::string ts_str;

        // Check top-level timestamp
        if (j.is_object() && j.contains("timestamp") && j["timestamp"].is_string())
        {
            ts_str = j["timestamp"].get<std::string>();
        }
        // Check nested in "result"
        else if (j.is_object() && j.contains("result") && j["result"].is_object() &&
                 j["result"].contains("timestamp") && j["result"]["timestamp"].is_string())
        {
            ts_str = j["result"]["timestamp"].get<std::string>();
        }

        if (!ts_str.empty())
        {
            std::string readable = format_timestamp(ts_str);
            std::cout << "  [TIMESTAMP] " << ts_str << " = " << readable << "\n";
        }
        else if (j.is_object())
        {
            std::cout << "  [TIMESTAMP] missing timestamp\n";
        }
    }
    catch (...)
    {
    }
}

// Debug macro for logging JSON before/after operations
#define JSON_DEBUG(label, j)                      \
    do                                            \
    {                                             \
        display_timestamp_if_present((j));        \
        std::cout << "[DEBUG] " << label << ":\n" \
                  << (j).dump(2) << std::endl;    \
    } while (0)
