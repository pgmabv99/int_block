// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "forge-std/Test.sol";
import "../src/NFTOwnerTracker.sol";

contract NFTOwnerTrackerTest is Test {
    // ============ Events ============

    event NFTRegistered(uint256 indexed tokenId, address indexed owner);
    event NFTTransferred(uint256 indexed tokenId, address indexed previousOwner, address indexed newOwner);
    event AdminChanged(address indexed previousAdmin, address indexed newAdmin);

    // ============ State ============

    NFTOwnerTracker nftTracker;
    address admin;
    address owner1;
    address owner2;
    address owner3;

    // ============ Setup ============

    function setUp() public {
        admin = address(this);
        owner1 = address(0x1111);
        owner2 = address(0x2222);
        owner3 = address(0x3333);

        nftTracker = new NFTOwnerTracker();
    }

    // ============ Tests: Registration ============

    function testRegisterNFT() public {
        nftTracker.registerNFT(1, owner1);

        assertEq(nftTracker.getOwner(1), owner1);
        assertEq(nftTracker.totalTokens(), 1);
    }

    function testRegisterMultipleNFTs() public {
        nftTracker.registerNFT(1, owner1);
        nftTracker.registerNFT(2, owner1);
        nftTracker.registerNFT(3, owner2);

        assertEq(nftTracker.getOwner(1), owner1);
        assertEq(nftTracker.getOwner(2), owner1);
        assertEq(nftTracker.getOwner(3), owner2);
        assertEq(nftTracker.totalTokens(), 3);
    }

    function testRegisterBatchNFTs() public {
        uint256[] memory tokenIds = new uint256[](3);
        tokenIds[0] = 1;
        tokenIds[1] = 2;
        tokenIds[2] = 3;

        nftTracker.registerBatchNFTs(tokenIds, owner1);

        assertEq(nftTracker.getOwner(1), owner1);
        assertEq(nftTracker.getOwner(2), owner1);
        assertEq(nftTracker.getOwner(3), owner1);
        assertEq(nftTracker.totalTokens(), 3);
    }

    function testRegisterBatchNFTsMultipleOwners() public {
        uint256[] memory batch1 = new uint256[](2);
        batch1[0] = 1;
        batch1[1] = 2;

        uint256[] memory batch2 = new uint256[](2);
        batch2[0] = 3;
        batch2[1] = 4;

        nftTracker.registerBatchNFTs(batch1, owner1);
        nftTracker.registerBatchNFTs(batch2, owner2);

        assertEq(nftTracker.getOwner(1), owner1);
        assertEq(nftTracker.getOwner(3), owner2);
        assertEq(nftTracker.totalTokens(), 4);
    }

    function testRegisterFailsForDuplicateToken() public {
        nftTracker.registerNFT(1, owner1);

        vm.expectRevert(NFTOwnerTracker.TokenAlreadyExists.selector);
        nftTracker.registerNFT(1, owner2);
    }

    function testRegisterFailsForZeroAddress() public {
        vm.expectRevert(NFTOwnerTracker.InvalidAddress.selector);
        nftTracker.registerNFT(1, address(0));
    }

    // ============ Tests: Query Functions ============

    function testGetOwnedTokens() public {
        nftTracker.registerNFT(1, owner1);
        nftTracker.registerNFT(2, owner1);
        nftTracker.registerNFT(3, owner2);

        uint256[] memory owner1Tokens = nftTracker.getOwnedTokens(owner1);
        assertEq(owner1Tokens.length, 2);
        assertEq(owner1Tokens[0], 1);
        assertEq(owner1Tokens[1], 2);

        uint256[] memory owner2Tokens = nftTracker.getOwnedTokens(owner2);
        assertEq(owner2Tokens.length, 1);
        assertEq(owner2Tokens[0], 3);
    }

    function testGetOwnedTokenCount() public {
        nftTracker.registerNFT(1, owner1);
        nftTracker.registerNFT(2, owner1);
        nftTracker.registerNFT(3, owner2);

        assertEq(nftTracker.getOwnedTokenCount(owner1), 2);
        assertEq(nftTracker.getOwnedTokenCount(owner2), 1);
        assertEq(nftTracker.getOwnedTokenCount(owner3), 0);
    }

    function testGetOwnedTokenAt() public {
        nftTracker.registerNFT(10, owner1);
        nftTracker.registerNFT(20, owner1);
        nftTracker.registerNFT(30, owner1);

        assertEq(nftTracker.getOwnedTokenAt(owner1, 0), 10);
        assertEq(nftTracker.getOwnedTokenAt(owner1, 1), 20);
        assertEq(nftTracker.getOwnedTokenAt(owner1, 2), 30);
    }

    function testIsTokenTracked() public {
        nftTracker.registerNFT(1, owner1);

        assertTrue(nftTracker.isTokenTracked(1));
        assertFalse(nftTracker.isTokenTracked(2));
    }

    function testGetOwnerFailsForNonExistentToken() public {
        vm.expectRevert(NFTOwnerTracker.TokenNotFound.selector);
        nftTracker.getOwner(999);
    }

    function testGetTokenInfo() public {
        nftTracker.registerNFT(1, owner1);

        address owner = nftTracker.getTokenInfo(1);
        assertEq(owner, owner1);
    }

    // ============ Tests: Transfer ============

    function testTransferNFT() public {
        nftTracker.registerNFT(1, owner1);

        nftTracker.transferNFT(1, owner2);

        assertEq(nftTracker.getOwner(1), owner2);
        assertEq(nftTracker.getOwnedTokenCount(owner1), 0);
        assertEq(nftTracker.getOwnedTokenCount(owner2), 1);
    }

    function testTransferMultipleNFTs() public {
        nftTracker.registerNFT(1, owner1);
        nftTracker.registerNFT(2, owner1);
        nftTracker.registerNFT(3, owner1);

        nftTracker.transferNFT(1, owner2);
        nftTracker.transferNFT(2, owner3);

        assertEq(nftTracker.getOwnedTokenCount(owner1), 1);
        assertEq(nftTracker.getOwnedTokenCount(owner2), 1);
        assertEq(nftTracker.getOwnedTokenCount(owner3), 1);
        assertEq(nftTracker.getOwner(3), owner1);
    }

    function testTransferNFTPreservesOrderAfterRemoval() public {
        nftTracker.registerNFT(1, owner1);
        nftTracker.registerNFT(2, owner1);
        nftTracker.registerNFT(3, owner1);

        // Transfer token 1 (first in array)
        nftTracker.transferNFT(1, owner2);

        // Verify remaining tokens are in correct order
        uint256[] memory remaining = nftTracker.getOwnedTokens(owner1);
        assertEq(remaining.length, 2);
        // After removal, last token (3) takes place of first token (1)
        assertEq(remaining[0], 3);
        assertEq(remaining[1], 2);
    }

    function testTransferFailsForNonExistentToken() public {
        vm.expectRevert(NFTOwnerTracker.TokenNotFound.selector);
        nftTracker.transferNFT(999, owner2);
    }

    function testTransferFailsForZeroAddress() public {
        nftTracker.registerNFT(1, owner1);

        vm.expectRevert(NFTOwnerTracker.InvalidAddress.selector);
        nftTracker.transferNFT(1, address(0));
    }

    function testTransferFailsForSameOwner() public {
        nftTracker.registerNFT(1, owner1);

        vm.expectRevert(NFTOwnerTracker.SameOwner.selector);
        nftTracker.transferNFT(1, owner1);
    }

    // ============ Tests: Admin ============

    function testInitialAdminIsDeployer() view public {
        assertEq(nftTracker.admin(), admin);
    }

    function testChangeAdmin() public {
        nftTracker.changeAdmin(owner1);
        assertEq(nftTracker.admin(), owner1);
    }

    function testChangeAdminFailsForNonAdmin() public {
        vm.prank(owner1);
        vm.expectRevert(NFTOwnerTracker.Unauthorized.selector);
        nftTracker.changeAdmin(owner2);
    }

    function testChangeAdminFailsForZeroAddress() public {
        vm.expectRevert(NFTOwnerTracker.InvalidAddress.selector);
        nftTracker.changeAdmin(address(0));
    }

    function testOperationsFailForNonAdmin() public {
        vm.prank(owner1);
        vm.expectRevert(NFTOwnerTracker.Unauthorized.selector);
        nftTracker.registerNFT(1, owner1);

        vm.prank(owner1);
        vm.expectRevert(NFTOwnerTracker.Unauthorized.selector);
        nftTracker.transferNFT(1, owner2);
    }

    // ============ Tests: Complex Scenarios ============

    function testComplexScenario() public {
        // Register tokens for multiple owners
        nftTracker.registerNFT(1, owner1);
        nftTracker.registerNFT(2, owner1);
        nftTracker.registerNFT(3, owner2);
        nftTracker.registerNFT(4, owner2);
        nftTracker.registerNFT(5, owner2);

        // Transfer some tokens
        nftTracker.transferNFT(1, owner3);
        nftTracker.transferNFT(3, owner1);

        // Verify state
        assertEq(nftTracker.getOwnedTokenCount(owner1), 2);
        assertEq(nftTracker.getOwnedTokenCount(owner2), 2);
        assertEq(nftTracker.getOwnedTokenCount(owner3), 1);
        assertEq(nftTracker.totalTokens(), 5);

        // Verify ownership
        assertEq(nftTracker.getOwner(1), owner3);
        assertEq(nftTracker.getOwner(3), owner1);
        assertEq(nftTracker.getOwner(5), owner2);
    }

    function testBatchOperationAndTransfer() public {
        uint256[] memory tokenIds = new uint256[](5);
        for (uint256 i = 0; i < 5; i++) {
            tokenIds[i] = i + 1;
        }

        nftTracker.registerBatchNFTs(tokenIds, owner1);
        assertEq(nftTracker.getOwnedTokenCount(owner1), 5);

        // Transfer odd tokens to owner2
        for (uint256 i = 1; i <= 5; i += 2) {
            nftTracker.transferNFT(i, owner2);
        }

        assertEq(nftTracker.getOwnedTokenCount(owner1), 2);
        assertEq(nftTracker.getOwnedTokenCount(owner2), 3);
    }

    function testEmptyOwnerTokenList() view public {
        uint256[] memory tokens = nftTracker.getOwnedTokens(owner1);
        assertEq(tokens.length, 0);
    }

    // ============ Tests: Events ============

    function testNFTRegisteredEvent() public {
        vm.expectEmit(true, true, false, false);
        emit NFTRegistered(1, owner1);

        nftTracker.registerNFT(1, owner1);
    }

    function testNFTTransferredEvent() public {
        nftTracker.registerNFT(1, owner1);

        vm.expectEmit(true, true, true, false);
        emit NFTTransferred(1, owner1, owner2);

        nftTracker.transferNFT(1, owner2);
    }

    function testAdminChangedEvent() public {
        vm.expectEmit(true, true, false, false);
        emit AdminChanged(admin, owner1);

        nftTracker.changeAdmin(owner1);
    }
}
