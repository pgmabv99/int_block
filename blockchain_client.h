#pragma once

#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <curl/curl.h>
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
