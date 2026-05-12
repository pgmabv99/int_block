# NFT Owner Tracker - Foundry Smart Contract Project

A comprehensive Solidity smart contract system to track and manage NFT ownership across different collections, built with Foundry.

## 📋 Overview

The **NFTOwnerTracker** contract provides a robust framework for:
- **Registering NFTs** - Add new NFTs with owner information
- **Tracking Ownership** - Query current and historical NFT owners
- **Managing Transfers** - Transfer NFTs between addresses
- **Batch Operations** - Efficiently process multiple tokens
- **Access Control** - Admin-only functions with role management

## 🏗️ Project Structure

```
foundry/
├── src/
│   └── NFTOwnerTracker.sol          # Main smart contract
├── test/
│   ├── NFTOwnerTracker.t.sol        # Comprehensive test suite (29 tests)
│   └── Counter.t.sol                # Example test (auto-generated)
├── script/
│   ├── Deploy.s.sol                 # Deployment script
│   └── Counter.s.sol                # Example script (auto-generated)
├── lib/
│   └── forge-std/                   # Foundry Standard Library
├── foundry.toml                     # Foundry configuration
└── README.md                        # This file
```

## 🚀 Quick Start

### Prerequisites
- **Foundry**: [Install here](https://book.getfoundry.sh/getting-started/installation)
- **Solidity 0.8.20+**

### Build the Project
```bash
cd /home/alexe/int_block/foundry
forge build
```

### Run Tests
```bash
forge test
```

### Run Specific Test
```bash
forge test --match testRegisterNFT
```

### Run Tests with Gas Report
```bash
forge test --gas-report
```

### Deploy Locally
```bash
forge script script/Deploy.s.sol --broadcast --rpc-url http://localhost:8545
```

## 📝 Contract Features

### Core Functions

#### Registration
- **`registerNFT(uint256 tokenId, address owner)`** - Register a single NFT
- **`registerBatchNFTs(uint256[] calldata tokenIds, address owner)`** - Register multiple NFTs in one transaction

#### Transfer
- **`transferNFT(uint256 tokenId, address newOwner)`** - Transfer NFT to a new owner

#### Queries
- **`getOwner(uint256 tokenId)`** - Get current owner of a token
- **`getOwnedTokens(address owner)`** - Get all tokens owned by an address
- **`getOwnedTokenCount(address owner)`** - Get number of tokens owned
- **`getOwnedTokenAt(address owner, uint256 index)`** - Get specific token by index
- **`isTokenTracked(uint256 tokenId)`** - Check if token is registered
- **`getTokenInfo(uint256 tokenId)`** - Get full token information

#### Admin
- **`changeAdmin(address newAdmin)`** - Transfer admin privileges

### Events
- **`NFTRegistered(uint256 indexed tokenId, address indexed owner)`** - Emitted on NFT registration
- **`NFTTransferred(uint256 indexed tokenId, address indexed previousOwner, address indexed newOwner)`** - Emitted on transfer
- **`AdminChanged(address indexed previousAdmin, address indexed newAdmin)`** - Emitted on admin change

### Access Control
- **Admin-only functions**: `registerNFT`, `registerBatchNFTs`, `transferNFT`, `changeAdmin`
- Initial admin is the contract deployer

## 🧪 Test Suite

The contract includes **29 comprehensive tests** covering:

### Registration Tests (6 tests)
- ✅ Single NFT registration
- ✅ Multiple NFT registration
- ✅ Batch registration
- ✅ Batch registration with multiple owners
- ✅ Duplicate token prevention
- ✅ Zero address validation

### Query Tests (7 tests)
- ✅ Get owned tokens
- ✅ Get token count
- ✅ Get token by index
- ✅ Track token existence
- ✅ Get token information
- ✅ Non-existent token handling
- ✅ Empty token list handling

### Transfer Tests (6 tests)
- ✅ Basic transfer
- ✅ Multiple transfers
- ✅ Array order preservation after removal
- ✅ Non-existent token handling
- ✅ Zero address validation
- ✅ Same owner prevention

### Admin Tests (4 tests)
- ✅ Initial admin verification
- ✅ Admin change
- ✅ Non-admin authorization check
- ✅ Zero address validation

### Complex Scenarios (2 tests)
- ✅ Multi-owner operations
- ✅ Batch operations with transfers

### Event Tests (3 tests)
- ✅ NFT registration event
- ✅ NFT transfer event
- ✅ Admin change event

**Test Results**: All 29 tests passing ✓

## 🛠️ Usage Examples

### Deploy the Contract
```solidity
NFTOwnerTracker tracker = new NFTOwnerTracker();
```

### Register an NFT
```solidity
tracker.registerNFT(1, 0x1234567890123456789012345678901234567890);
```

### Register Multiple NFTs
```solidity
uint256[] memory tokenIds = new uint256[](3);
tokenIds[0] = 1;
tokenIds[1] = 2;
tokenIds[2] = 3;
tracker.registerBatchNFTs(tokenIds, ownerAddress);
```

### Query Owner
```solidity
address owner = tracker.getOwner(1);
```

### Get Owned Tokens
```solidity
uint256[] memory tokens = tracker.getOwnedTokens(ownerAddress);
```

### Transfer NFT
```solidity
tracker.transferNFT(1, newOwnerAddress);
```

## 📊 Gas Efficiency

The contract uses optimized data structures:
- **Token tracking**: O(1) lookup using mapping
- **Owner tokens**: Array with index mapping for O(1) removal
- **Batch operations**: Reduced gas overhead compared to individual calls

## 🔒 Security Considerations

1. **Access Control**: Admin-only functions prevent unauthorized operations
2. **Input Validation**: Zero address checks and duplicate prevention
3. **State Consistency**: Transfer operations maintain data structure integrity
4. **Event Logging**: All state changes emit events for auditability

## 📚 Deployment Instructions

### Local Deployment with Anvil
```bash
# Terminal 1: Start Anvil local blockchain
anvil

# Terminal 2: Deploy contract
cd /home/alexe/int_block/foundry
forge script script/Deploy.s.sol --broadcast --rpc-url http://localhost:8545
```

### Testnet Deployment
```bash
forge script script/Deploy.s.sol --broadcast \
  --rpc-url https://sepolia.infura.io/v3/YOUR_INFURA_KEY \
  --private-key YOUR_PRIVATE_KEY
```

## 🔍 Verification

### Verify Contract on Etherscan
```bash
forge verify-contract \
  --chain sepolia \
  --watch 0xYourContractAddress \
  src/NFTOwnerTracker.sol:NFTOwnerTracker \
  --verifier-url https://api-sepolia.etherscan.io/api
```

## 📖 Foundry Documentation

- [Foundry Book](https://book.getfoundry.sh/)
- [Forge Command Reference](https://book.getfoundry.sh/reference/forge/)
- [Solidity Documentation](https://docs.soliditylang.org/)

## 🎯 Next Steps

Possible enhancements:
- [ ] ERC721 integration for real NFT contracts
- [ ] Multi-collection support
- [ ] Batch transfer operations
- [ ] Burn functionality
- [ ] Metadata storage
- [ ] Upgradeable proxy pattern

## 📄 License

SPDX-License-Identifier: MIT

---

**Project Path**: `/home/alexe/int_block/foundry/`

**Solidity Version**: 0.8.20

**Foundry Version**: v1.7.1
$ forge script script/Counter.s.sol:CounterScript --rpc-url <your_rpc_url> --private-key <your_private_key>
```

### Cast

```shell
$ cast <subcommand>
```

### Help

```shell
$ forge --help
$ anvil --help
$ cast --help
```
