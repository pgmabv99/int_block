#!/bin/bash
# run.sh - Call various functions on the deployed NFTOwnerTracker contract

echo "📋 NFTOwnerTracker Contract Interactions"
echo "Contract: $(cat broadcast/LATEST_DEPLOYMENT.txt)"
echo "RPC: http://localhost:8545"
echo "=================================================="
echo ""

# Call 1: Get admin address
echo "1️⃣  Admin Address:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "admin()" --rpc-url http://localhost:8545
echo ""

# Call 2: Get total tokens tracked
echo "2️⃣  Total Tokens Tracked:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "totalTokens()" --rpc-url http://localhost:8545
echo ""

# Call 3: Get owned token count for an address (will be 0 initially)
echo "3️⃣  Owned Token Count for 0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "getOwnedTokenCount(address)" 0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266 --rpc-url http://localhost:8545
echo ""

# Call 4: Check if a specific token is tracked
echo "4️⃣  Is Token 1 Tracked:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "isTokenTracked(uint256)" 1 --rpc-url http://localhost:8545
echo ""

# Call 5: Try to get owner of non-existent token (will show address(0))
echo "5️⃣  Owner of Token 1:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "getOwner(uint256)" 1 --rpc-url http://localhost:8545
echo ""

# Call 6: Register a token
echo "6️⃣  Register Token 1 for 0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266:"
cast send $(cat broadcast/LATEST_DEPLOYMENT.txt) "registerNFT(uint256,address)" 1 0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266 --rpc-url http://localhost:8545 --private-key 0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80
echo "   ⛽ Gas Cost: ~117,611 gas @ 785,206,958 wei/gas ≈ 0.0000923 ETH (~$0.23 @ $2500/ETH)"
echo ""

# Call 7: Verify token is now tracked
echo "7️⃣  Verify Token 1 is Tracked:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "isTokenTracked(uint256)" 1 --rpc-url http://localhost:8545
echo ""

# Call 8: Check owner of registered token
echo "8️⃣  Check Owner of Token 1:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "getOwner(uint256)" 1 --rpc-url http://localhost:8545
echo ""

# Call 9: Check total tokens
echo "9️⃣  Total Tokens Tracked:"
cast call $(cat broadcast/LATEST_DEPLOYMENT.txt) "totalTokens()" --rpc-url http://localhost:8545
echo ""

echo "=================================================="
echo "✅ Contract calls completed!"
echo ""
echo "📊 Gas Price Reference:"
echo "   gasUsed: 117,611"
echo "   effectiveGasPrice: 785,206,958 wei/gas"
echo "   Total: 117,611 × 785,206,958 = 92,324,265,961,538 wei"
echo "   In ETH: 92,324,265,961,538 ÷ 10^18 ≈ 0.0000923 ETH"
echo "   In USD: 0.0000923 × $2,500 ≈ $0.23"
echo "   Note: Actual cost varies with ETH price and network gas conditions"
