#include <iostream>
#include <cstdlib>
#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>
#include <cmath>
#include "blockchain_client.h"

// Helper function to convert hex string to decimal string
std::string hexToDecimal(const std::string &hex)
{
    std::string hexStr = hex;
    if (hexStr.substr(0, 2) == "0x" || hexStr.substr(0, 2) == "0X")
    {
        hexStr = hexStr.substr(2);
    }

    // Use simple conversion for demonstration
    // For large numbers, we'd need a big integer library
    unsigned long long value = 0;
    try
    {
        value = std::stoull(hexStr, nullptr, 16);
    }
    catch (...)
    {
        return "0";
    }
    return std::to_string(value);
}

// Helper function to convert wei to ETH
double weiToEth(const std::string &weiStr)
{
    try
    {
        unsigned long long wei = std::stoull(weiStr);
        return static_cast<double>(wei) / 1e18;
    }
    catch (...)
    {
        return 0.0;
    }
}

// Helper function to get current ETH price in USD (using a reasonable estimate)
double getEthPriceUSD()
{
    // For demonstration, returning a reasonable ETH price
    // In production, you'd call CoinGecko API or similar
    return 2500.0; // Approximate ETH price in USD
}

int main()
{
    try
    {
        // Get API key from environment variable
        const char *apiKey = std::getenv("ALCHEMY_API_KEY");
        if (!apiKey)
        {
            std::cerr << "Error: ALCHEMY_API_KEY environment variable not set.\n";
            std::cerr << "Get a free key at: https://www.alchemy.com/\n";
            std::cerr << "Set it with: export ALCHEMY_API_KEY=your_alchemy_key\n";
            return 1;
        }

        BlockchainClient client(apiKey);

        std::cout << "=== Ethereum Blockchain Query Sample ===\n\n";

        // Query 1: Current block number
        std::cout << "1. Current Block Number:\n";
        auto blockNumber = client.getBlockNumber();
        std::cout << "   Block #: " << blockNumber << "\n\n";

        // Query 2: Block details
        std::cout << "2. Latest Block Details:\n";
        auto block = client.getBlockByNumber("latest");
        std::cout << "   Miner: " << block["miner"] << "\n";
        std::cout << "   Timestamp: " << block["timestamp"] << "\n";
        std::cout << "   Transactions: " << block["transactions"].size() << "\n\n";

        // Query 3: Account balance (Vitalik Buterin's address as example)
        std::cout << "3. Account Balance (Vitalik):\n";
        std::string vitalikAddress = "0xd8dA6BF26964aF9D7eEd9e03E53415D37aA96045";
        auto balance = client.getBalance(vitalikAddress);

        // Convert hex balance to decimal wei
        std::string balanceHex = balance.is_string() ? balance.get<std::string>() : balance.dump();
        std::string balanceWei = hexToDecimal(balanceHex);

        // Convert wei to ETH
        double balanceEth = weiToEth(balanceWei);

        // Convert to USD
        double ethPrice = getEthPriceUSD();
        double balanceUsd = balanceEth * ethPrice;

        // Display all three formats
        std::cout << "   Balance (wei): " << balanceWei << "\n";
        std::cout << "   Balance (ETH): " << std::fixed << std::setprecision(6) << balanceEth << " ETH\n";
        std::cout << "   Balance (USD): $" << std::fixed << std::setprecision(2) << balanceUsd << "\n\n";

        // Query 4: Gas price
        std::cout << "4. Current Gas Price:\n";
        auto gasPrice = client.getGasPrice();
        std::cout << "   Gas Price (wei): " << gasPrice << "\n\n";

        // Query 5: Transaction count for address
        std::cout << "5. Transaction Count:\n";
        auto txCount = client.getTransactionCount(vitalikAddress);
        std::cout << "   Vitalik's transactions: " << txCount << "\n\n";

        // return 0;
        // Query 6: Listen to pending transactions (demo - runs for 10 seconds)
        std::cout << "6. Listening to Pending Transactions (10 seconds):\n";
        std::cout << "   (Requires Infura paid plan or alternative provider)\n\n";

        client.pending_transaction_count = 0;

        // Caller manages thread creation and validation
        if (client.streaming)
        {
            std::cout << "   Stream already running!\n\n";
        }
        else
        {
            try
            {
                client.streaming = true;
                auto callback = std::bind(&BlockchainClient::onPendingTransaction, &client, std::placeholders::_1);
                client.stream_thread = std::thread(&BlockchainClient::streamTransactionWorker, &client, callback);

                std::this_thread::sleep_for(std::chrono::seconds(10));

                client.streaming = false;
                if (client.stream_thread.joinable())
                {
                    client.stream_thread.join();
                }
                std::cout << "   Total pending transactions seen: " << client.pending_transaction_count << "\n\n";
            }
            catch (const std::exception &e)
            {
                std::cout << "   Transaction stream error: " << e.what() << "\n\n";
                client.streaming = false;
            }
        }

        std::cout << "✓ All queries completed successfully!\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
