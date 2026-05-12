#include <iostream>
#include <cstdlib>
#include <chrono>
#include <functional>
#include "blockchain_client.h"

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
        // Convert hex balance to ETH (divide by 10^18)
        std::cout << "   Raw balance (wei): " << balance << "\n\n";

        // Query 4: Gas price
        std::cout << "4. Current Gas Price:\n";
        auto gasPrice = client.getGasPrice();
        std::cout << "   Gas Price (wei): " << gasPrice << "\n\n";

        // Query 5: Transaction count for address
        std::cout << "5. Transaction Count:\n";
        auto txCount = client.getTransactionCount(vitalikAddress);
        std::cout << "   Vitalik's transactions: " << txCount << "\n\n";

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
