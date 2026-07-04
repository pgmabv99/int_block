#include "blockchain_client.h"
#include <iostream>
#include <cstring>

size_t BlockchainClient::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    userp->append((char *)contents, size * nmemb);
    return size * nmemb;
}

BlockchainClient::BlockchainClient(const std::string &alchemy_api_key)
{
    // Using Alchemy instead of Infura (free tier supports WebSocket)
    endpoint = "https://eth-mainnet.g.alchemy.com/v2/" + alchemy_api_key;
    ws_endpoint = "wss://eth-mainnet.g.alchemy.com/v2/" + alchemy_api_key;
}

json BlockchainClient::makeRequest(const std::string &method, const json &params)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Failed to initialize CURL\n";
        throw std::runtime_error("Failed to initialize CURL");
    }

    // Prepare JSON-RPC payload
    json payload;
    payload["jsonrpc"] = "2.0";
    payload["method"] = method;
    payload["params"] = params;
    payload["id"] = 1;
    JSON_DEBUG("Payload before CURL call", payload);

    std::string payload_str = payload.dump();
    std::string responseStr;

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    // Perform request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Parse response
    try
    {
        auto response = json::parse(responseStr);
        JSON_DEBUG("Response after CURL call", response);
        if (response.contains("error"))
        {
            throw std::runtime_error("RPC Error: " + response["error"].dump());
        }
        return response["result"];
    }
    catch (const json::exception &e)
    {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()) + "\nResponse: " + responseStr);
    }
}

json BlockchainClient::getBlockNumber()
{
    return makeRequest("eth_blockNumber", json::array());
}

json BlockchainClient::getBlockByNumber(const std::string &blockNumber)
{
    // return makeRequest("eth_getBlockByNumber", json::array({blockNumber, false}));
    json xx = json::array();
    xx.push_back(blockNumber);
    xx.push_back(false);
    return makeRequest("eth_getBlockByNumber", xx);
}

json BlockchainClient::getBalance(const std::string &address)
{
    return makeRequest("eth_getBalance", json::array({address, "latest"}));
}

json BlockchainClient::getTransactionByHash(const std::string &txHash)
{
    return makeRequest("eth_getTransactionByHash", json::array({txHash}));
}

json BlockchainClient::getGasPrice()
{
    return makeRequest("eth_gasPrice", json::array());
}

json BlockchainClient::getTransactionCount(const std::string &address)
{
    return makeRequest("eth_getTransactionCount", json::array({address, "latest"}));
}

void BlockchainClient::streamTransactionWorker(TransactionCallback callback)
{
    try
    {
        // Initialize WebSocket TLS client
        ws_client = std::make_shared<WSClient>();
        tx_callback = callback;

        // Set up handlers
        ws_client->set_open_handler(std::bind(&BlockchainClient::on_open, this, std::placeholders::_1));
        ws_client->set_close_handler(std::bind(&BlockchainClient::on_close, this, std::placeholders::_1));
        ws_client->set_message_handler(std::bind(&BlockchainClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
        ws_client->set_fail_handler(std::bind(&BlockchainClient::on_fail, this, std::placeholders::_1));

        // Set TLS init handler for WSS connections
        ws_client->set_tls_init_handler([](websocketpp::connection_hdl hdl)
                                        {
            auto ctx = websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(
                websocketpp::lib::asio::ssl::context::sslv23);
            ctx->set_options(
                websocketpp::lib::asio::ssl::context::default_workarounds |
                websocketpp::lib::asio::ssl::context::no_sslv2 |
                websocketpp::lib::asio::ssl::context::single_dh_use);
            // For testing: disable certificate verification
            ctx->set_verify_mode(websocketpp::lib::asio::ssl::verify_none);
            return ctx; });

        // Initialize ASIO
        ws_client->init_asio();

        // Create connection
        websocketpp::lib::error_code ec;
        WSClient::connection_ptr con = ws_client->get_connection(ws_endpoint, ec);

        if (ec)
        {
            std::cerr << "Connection init error: " << ec.message() << std::endl;
            return;
        }

        ws_hdl = con->get_handle();
        ws_client->connect(con);

        // Run the event loop
        ws_client->run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "WebSocket worker error: " << e.what() << std::endl;
    }
}

void BlockchainClient::on_open(ConnectionHandle hdl)
{
    std::cout << "   WebSocket connected!\n";

    // Subscribe to pending transactions
    json subscribe_msg;
    subscribe_msg["jsonrpc"] = "2.0";
    subscribe_msg["id"] = 1;
    subscribe_msg["method"] = "eth_subscribe";
    subscribe_msg["params"] = json::array({"newPendingTransactions"});
    JSON_DEBUG("WebSocket subscribe message", subscribe_msg);

    try
    {
        ws_client->send(hdl, subscribe_msg.dump(), websocketpp::frame::opcode::text);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Send error: " << e.what() << std::endl;
    }
}

void BlockchainClient::on_close(ConnectionHandle hdl)
{
    std::cout << "   WebSocket closed\n";
}

void BlockchainClient::on_message(ConnectionHandle hdl, WSClient::message_ptr msg)
{
    try
    {
        auto response = json::parse(msg->get_payload());
        JSON_DEBUG("WebSocket message received", response);

        // Handle subscription confirmation
        if (response.contains("result"))
        {
            std::cout << "   Subscription ID: " << response["result"] << "\n";
            return;
        }

        // Handle transaction notifications
        if (response.contains("params") && response["params"].contains("result"))
        {
            auto tx_hash = response["params"]["result"];
            if (tx_callback)
            {
                tx_callback(tx_hash);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Message parsing error: " << e.what() << std::endl;
    }
}

void BlockchainClient::on_fail(ConnectionHandle hdl)
{
    std::cerr << "   WebSocket connection failed\n";
}

BlockchainClient::~BlockchainClient()
{
    streaming = false;
    if (ws_client)
    {
        ws_client->stop();
    }
    if (stream_thread.joinable())
    {
        stream_thread.join();
    }
}

void BlockchainClient::onPendingTransaction(const json &tx)
{
    try
    {
        // tx is just the hash string, fetch full details
        std::string txHash = tx.get<std::string>();
        auto txData = getTransactionByHash(txHash);
        JSON_DEBUG("Transaction data fetched", txData);

        std::cout << "\n   New Transaction:\n";
        std::cout << "   Hash:     " << txHash << "\n";

        if (txData.contains("from"))
            std::cout << "   From:     " << txData["from"].get<std::string>() << "\n";

        if (txData.contains("to") && !txData["to"].is_null())
            std::cout << "   To:       " << txData["to"].get<std::string>() << "\n";
        else
            std::cout << "   To:       (contract creation)\n";

        if (txData.contains("value"))
        {
            // Convert wei to ETH
            std::string valueHex = txData["value"].get<std::string>();
            std::cout << "   Value:    " << valueHex << " wei\n";
        }

        if (txData.contains("gasPrice"))
            std::cout << "   Gas Price: " << txData["gasPrice"].get<std::string>() << "\n";

        if (txData.contains("gas"))
            std::cout << "   Gas Limit: " << txData["gas"].get<std::string>() << "\n";

        if (txData.contains("nonce"))
            std::cout << "   Nonce:    " << txData["nonce"].get<std::string>() << "\n";

        if (txData.contains("type"))
            std::cout << "   Type:     " << txData["type"].get<std::string>() << "\n";

        // Smart contract data
        if (txData.contains("input"))
        {
            std::string input = txData["input"].get<std::string>();
            if (input != "0x") // "0x" means no contract call data
            {
                if (input.length() >= 10)
                {
                    std::string selector = input.substr(0, 10);
                    std::string funcName = decodeFunctionSelector(selector);
                    std::cout << "   Function: " << selector << " (" << funcName << ")\n";
                    std::cout << "   Input:    " << input << "\n";
                }
            }
            else
            {
                std::cout << "   Type:     Regular ETH transfer (no contract call)\n";
            }
        }

        std::cout << std::flush;
        pending_transaction_count++;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching transaction details: " << e.what() << "\n";
    }
}

std::string BlockchainClient::decodeFunctionSelector(const std::string &selector)
{
    try
    {
        // Query 4byte.directory API
        CURL *curl = curl_easy_init();
        if (!curl)
            return selector + " (unknown)";

        std::string url = "https://www.4byte.directory/api/v1/signatures/?hex_signature=" + selector;
        std::string responseStr;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L); // Short timeout

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
            return selector + " (lookup failed)";

        // Parse response
        auto response = json::parse(responseStr);
        JSON_DEBUG("4byte.directory response", response);
        if (response.contains("results") && response["results"].is_array() && response["results"].size() > 0)
        {
            auto results = response["results"][0];
            if (results.contains("text_signature"))
            {
                return results["text_signature"].get<std::string>();
            }
        }
        return selector + " (unknown)";
    }
    catch (const std::exception &e)
    {
        return selector + " (error)";
    }
}
