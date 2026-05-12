// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "forge-std/Script.sol";
import "../src/NFTOwnerTracker.sol";

contract DeployNFTOwnerTracker is Script {
    function setUp() public {}

    function run() public {
        vm.startBroadcast();

        NFTOwnerTracker nftTracker = new NFTOwnerTracker();

        vm.stopBroadcast();

        // Log the deployed address
        console.log("NFTOwnerTracker deployed at:", address(nftTracker));
        console.log("Admin:", nftTracker.admin());
    }
}
