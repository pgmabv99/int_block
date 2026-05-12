# C++ Ethereum Blockchain Query Program

Sample program to query Ethereum blockchain data using Infura's JSON-RPC API with transaction stream monitoring.

## Prerequisites

1. **libcurl** - Install development headers:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libcurl4-openssl-dev
   
   # macOS
   brew install curl
   ```

2. **CMake 3.10+**
   ```bash
   sudo apt-get install cmake  # Linux
   brew install cmake          # macOS
   ```

3. **Infura API Key** - Get free at https://infura.io
   - Sign up (free, no credit card needed)
   - Create a new project
   - Copy your Project ID

## Setup & Build

1. **Download nlohmann/json header**:
   ```bash
   cd /home/alexe/int_block
   curl -o json.hpp https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
   ```

2. **Build the project**:
   ```bash
   cd /home/alexe/int_block
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Run the program**:
   ```bash
   export INFURA_API_KEY=your_infura_api_key_here
   ./blockchain_query
   ```

## Example Output

```
=== Ethereum Blockchain Query Sample ===

1. Current Block Number:
   Block #: 0x12a4d8f

2. Latest Block Details:
   Miner: 0x1111111254fb6c44bac0bed2854e76f90643097d
   Timestamp: 0x66f4a1c3
   Transactions: 145

3. Account Balance (Vitalik):
   Raw balance (wei): 0x1234...

4. Current Gas Price:
   Gas Price (wei): 0x5f5e100

5. Transaction Count:
   Vitalik's transactions: 0x4a2b

6. Listening to Pending Transactions (10 seconds):
   New pending transaction: 0x1a2b3c...
   Total pending transactions seen: 15
```

## Files

- `blockchain_client.h` - BlockchainClient class declaration
- `blockchain_client.cpp` - BlockchainClient implementation (JSON-RPC calls + transaction streaming)
- `main.cpp` - Sample usage with various queries and transaction monitoring
- `CMakeLists.txt` - Build configuration
- `json.hpp` - nlohmann/json library (download with curl command above)

## Supported Queries

The `BlockchainClient` class provides methods for:

**REST API Methods:**
- `getBlockNumber()` - Get latest block number
- `getBlockByNumber(blockNumber)` - Get block details
- `getBalance(address)` - Get ETH balance of an address
- `getTransactionByHash(txHash)` - Get transaction details
- `getGasPrice()` - Get current gas price
- `getTransactionCount(address)` - Get transaction count for an address

**Transaction Stream (Polling-based):**
- `startTransactionStream(callback)` - Start listening for pending transactions
- `stopTransactionStream()` - Stop the stream

## WebSocket Transaction Streaming

The transaction stream now uses **true WebSocket** push notifications instead of polling:

**How it works:**
1. Connects to Infura's WebSocket endpoint (`wss://mainnet.infura.io/ws/...`)
2. Subscribes to `eth_subscribe("newPendingTransactions")`
3. Receives real-time transaction hashes as they enter the mempool
4. Calls your callback function for each new transaction

**Usage:**
```cpp
auto callback = std::bind(&BlockchainClient::onPendingTransaction, &client, std::placeholders::_1);

client.resetTransactionCount();
client.startTransactionStream(callback);
std::this_thread::sleep_for(std::chrono::seconds(10));
client.stopTransactionStream();
std::cout << "Transactions: " << client.getPendingTransactionCount() << std::endl;
```

**Advantages:**
- Real-time push notifications (no polling delay)
- Lower latency (immediate transaction notification)
- Fewer API requests
- Better for high-frequency monitoring

**Note:**
- Requires Boost library (installed as dependency of websocketpp)
- Uses OpenSSL for WSS (WebSocket Secure) connections
- Free Infura tier may have connection limits - consider upgrading for production use

## Notes

- API key is read from `INFURA_API_KEY` environment variable
- Uses Ethereum mainnet by default
- Change endpoint in `BlockchainClient` constructor to use testnet or other chains
- JSON responses are returned as `nlohmann::json` objects
- Transaction streaming runs in a background thread with callback-based delivery

## Testing

Replace `INFURA_API_KEY` with your actual key and run:
```bash
export INFURA_API_KEY=abc123def456
./blockchain_query
```

## Debugging in VS Code

Press `F5` to debug with breakpoints. The program is built with debug symbols (`-g -O0`) for full debugging support.
