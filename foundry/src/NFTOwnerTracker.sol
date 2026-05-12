// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

/**
 * @title NFTOwnerTracker
 * @dev A smart contract to track NFT owners across different collections
 * @notice This contract manages ownership records of NFTs and provides query functions
 */
contract NFTOwnerTracker {
    // ============ State Variables ============

    /// @dev Admin address with special privileges
    address public admin;

    /// @dev Mapping from tokenId to owner address
    mapping(uint256 => address) private tokenOwners;

    /// @dev Mapping from owner address to array of owned token IDs
    mapping(address => uint256[]) private ownedTokens;

    /// @dev Mapping to track token index in owner's token array for efficient removal
    mapping(uint256 => uint256) private tokenIndex;

    /// @dev Total number of unique NFTs tracked
    uint256 public totalTokens;

    // ============ Events ============

    /// @dev Emitted when a new NFT is registered
    event NFTRegistered(uint256 indexed tokenId, address indexed owner);

    /// @dev Emitted when NFT ownership is transferred
    event NFTTransferred(uint256 indexed tokenId, address indexed previousOwner, address indexed newOwner);

    /// @dev Emitted when admin is changed
    event AdminChanged(address indexed previousAdmin, address indexed newAdmin);

    // ============ Errors ============

    error Unauthorized();
    error TokenNotFound();
    error InvalidAddress();
    error TokenAlreadyExists();
    error SameOwner();

    // ============ Constructor ============

    constructor() {
        admin = msg.sender;
    }

    // ============ Modifiers ============

    /// @dev Restricts function execution to admin only
    modifier onlyAdmin() {
        if (msg.sender != admin) revert Unauthorized();
        _;
    }

    // ============ Admin Functions ============

    /**
     * @dev Change the admin address
     * @param newAdmin The address of the new admin
     */
    function changeAdmin(address newAdmin) external onlyAdmin {
        if (newAdmin == address(0)) revert InvalidAddress();
        address previousAdmin = admin;
        admin = newAdmin;
        emit AdminChanged(previousAdmin, newAdmin);
    }

    // ============ Registration Functions ============

    /**
     * @dev Register a new NFT with its owner
     * @param tokenId The unique identifier of the NFT
     * @param owner The address of the owner
     */
    function registerNFT(uint256 tokenId, address owner) external onlyAdmin {
        if (owner == address(0)) revert InvalidAddress();
        if (tokenOwners[tokenId] != address(0)) revert TokenAlreadyExists();

        tokenOwners[tokenId] = owner;
        tokenIndex[tokenId] = ownedTokens[owner].length;
        ownedTokens[owner].push(tokenId);
        totalTokens++;

        emit NFTRegistered(tokenId, owner);
    }

    /**
     * @dev Register multiple NFTs at once (batch operation)
     * @param tokenIds Array of token IDs
     * @param owner The owner address for all tokens
     */
    function registerBatchNFTs(uint256[] calldata tokenIds, address owner) external onlyAdmin {
        if (owner == address(0)) revert InvalidAddress();

        for (uint256 i = 0; i < tokenIds.length; i++) {
            uint256 tokenId = tokenIds[i];
            if (tokenOwners[tokenId] != address(0)) revert TokenAlreadyExists();

            tokenOwners[tokenId] = owner;
            tokenIndex[tokenId] = ownedTokens[owner].length;
            ownedTokens[owner].push(tokenId);
            totalTokens++;

            emit NFTRegistered(tokenId, owner);
        }
    }

    // ============ Transfer Functions ============

    /**
     * @dev Transfer NFT ownership from one address to another
     * @param tokenId The token ID to transfer
     * @param newOwner The address of the new owner
     */
    function transferNFT(uint256 tokenId, address newOwner) external  onlyAdmin {
        if (newOwner == address(0)) revert InvalidAddress();

        address currentOwner = tokenOwners[tokenId];
        if (currentOwner == address(0)) revert TokenNotFound();
        if (currentOwner == newOwner) revert SameOwner();

        // Remove token from current owner's list
        uint256 index = tokenIndex[tokenId];
        uint256 lastToken = ownedTokens[currentOwner][ownedTokens[currentOwner].length - 1];

        ownedTokens[currentOwner][index] = lastToken;
        tokenIndex[lastToken] = index;
        ownedTokens[currentOwner].pop();

        // Add token to new owner's list
        tokenIndex[tokenId] = ownedTokens[newOwner].length;
        ownedTokens[newOwner].push(tokenId);
        tokenOwners[tokenId] = newOwner;

        emit NFTTransferred(tokenId, currentOwner, newOwner);
    }

    // ============ Query Functions ============

    /**
     * @dev Get the owner of a specific token
     * @param tokenId The token ID
     * @return The owner's address
     */
    function getOwner(uint256 tokenId) external view returns (address) {
        address owner = tokenOwners[tokenId];
        if (owner == address(0)) revert TokenNotFound();
        return owner;
    }

    /**
     * @dev Check if a token is tracked
     * @param tokenId The token ID
     * @return True if token exists, false otherwise
     */
    function isTokenTracked(uint256 tokenId) external view returns (bool) {
        return tokenOwners[tokenId] != address(0);
    }

    /**
     * @dev Get all tokens owned by an address
     * @param owner The owner's address
     * @return Array of token IDs owned by the address
     */
    function getOwnedTokens(address owner) external view returns (uint256[] memory) {
        return ownedTokens[owner];
    }

    /**
     * @dev Get the number of tokens owned by an address
     * @param owner The owner's address
     * @return The count of tokens owned
     */
    function getOwnedTokenCount(address owner) external view returns (uint256) {
        return ownedTokens[owner].length;
    }

    /**
     * @dev Get a specific token owned by an address
     * @param owner The owner's address
     * @param index The index in the owner's token list
     * @return The token ID at the specified index
     */
    function getOwnedTokenAt(address owner, uint256 index) external view returns (uint256) {
        return ownedTokens[owner][index];
    }

    /**
     * @dev Get all information about a token
     * @param tokenId The token ID
     * @return owner The owner's address
     */
    function getTokenInfo(uint256 tokenId) external view returns (address owner) {
        owner = tokenOwners[tokenId];
        if (owner == address(0)) revert TokenNotFound();
    }
}
