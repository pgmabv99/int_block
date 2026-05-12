#!/bin/bash
# Simple deploy script - saves contract address to file

PRIVATE_KEY="0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80"
RPC_URL="http://localhost:8545"

echo "🚀 Deploying NFTOwnerTracker..."

# Run deployment and capture output
OUTPUT=$(forge script script/Deploy.s.sol --broadcast \
  --rpc-url $RPC_URL \
  --private-key $PRIVATE_KEY 2>&1)

# Extract address from console logs
CONTRACT_ADDRESS=$(echo "$OUTPUT" | grep "NFTOwnerTracker deployed at:" | tail -1 | grep -oP '0x[a-fA-F0-9]{40}')

if [ -z "$CONTRACT_ADDRESS" ]; then
    echo "❌ Failed to extract address"
    exit 1
fi

# Save to file
echo "$CONTRACT_ADDRESS" > broadcast/LATEST_DEPLOYMENT.txt

echo "✅ Deployed at: $CONTRACT_ADDRESS"
echo "✅ Saved to: broadcast/LATEST_DEPLOYMENT.txt"
