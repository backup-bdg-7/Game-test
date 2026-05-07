// server.js - Doomnite Game Server with MongoDB
// Handles user authentication, player data sync, and progress storage
// Uses MongoDB for better scalability and flexible data handling

const express = require('express');
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const mongoose = require('mongoose');
const cors = require('cors');
const helmet = require('helmet');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;
const JWT_SECRET = process.env.JWT_SECRET || 'doomnite-secret-key-change-in-production';
const MONGODB_URI = process.env.MONGODB_URI || 'mongodb://localhost:27017/doomnite';

// Middleware
app.use(helmet()); // Security headers
app.use(cors()); // Enable CORS for iOS app
app.use(express.json());

// MongoDB connection
mongoose.connect(MONGODB_URI).then(() => {
    console.log('Connected to MongoDB successfully');
}).catch((error) => {
    console.error('MongoDB connection error:', error);
    process.exit(1);
});

// Mongoose Schemas

// User schema
const UserSchema = new mongoose.Schema({
    username: { type: String, unique: true, required: true },
    email: { type: String, unique: true, sparse: true },
    passwordHash: { type: String, required: true },
    createdAt: { type: Date, default: Date.now },
    lastLogin: { type: Date }
});

// Player profile schema
const PlayerProfileSchema = new mongoose.Schema({
    userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true },
    displayName: String,
    iconId: { type: String, default: 'default_1' },
    customPhotoPath: String,
    showHints: { type: Boolean, default: true },
    enableTutorial: { type: Boolean, default: true }
});

// Player progress schema (includes inventory, unlocks, achievements)
const PlayerProgressSchema = new mongoose.Schema({
    userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true, unique: true },
    level: { type: Number, default: 1 },
    currentXP: { type: Number, default: 0 },
    xpToNextLevel: { type: Number, default: 100 },
    skillPoints: { type: Number, default: 0 },
    totalKills: { type: Number, default: 0 },
    totalDeaths: { type: Number, default: 0 },
    totalPlayTime: { type: Number, default: 0 },
    totalQuestsCompleted: { type: Number, default: 0 },
    lastSaved: { type: Date, default: Date.now },
    // Inventory array
    inventory: [{
        itemId: String,
        itemType: String, // Weapon, Armor, Accessory, etc.
        slot: String, // helmet, chest, weapon, etc.
        isEquipped: { type: Boolean, default: false },
        durability: { type: Number, default: 100 },
        stats: mongoose.Schema.Types.Mixed // Flexible stats object
    }],
    // Unlocked content array
    unlockedContent: [{
        contentId: String,
        contentType: String,
        unlockedAt: { type: Date, default: Date.now }
    }],
    // Achievements array
    achievements: [{
        achievementId: String,
        progress: { type: Number, default: 0 },
        isCompleted: { type: Boolean, default: false },
        completedAt: Date
    }]
});

// Create models
const User = mongoose.model('User', UserSchema);
const PlayerProfile = mongoose.model('PlayerProfile', PlayerProfileSchema);
const PlayerProgress = mongoose.model('PlayerProgress', PlayerProgressSchema);

// Authentication middleware
function authenticateToken(req, res, next) {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1];
    
    if (!token) {
        return res.status(401).json({ error: 'Access token required' });
    }
    
    jwt.verify(token, JWT_SECRET, (err, user) => {
        if (err) {
            return res.status(403).json({ error: 'Invalid or expired token' });
        }
        req.user = user;
        next();
    });
}

// ROUTES

// Health check
app.get('/health', (req, res) => {
    res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

// Register new user
app.post('/api/auth/register', async (req, res) => {
    const { username, email, password } = req.body;
    
    if (!username || !password) {
        return res.status(400).json({ error: 'Username and password required' });
    }
    
    try {
        // Check if user exists
        const existingUser = await User.findOne({ $or: [{ username }, { email }] });
        if (existingUser) {
            return res.status(409).json({ error: 'Username or email already exists' });
        }
        
        // Hash password
        const salt = await bcrypt.genSalt(10);
        const passwordHash = await bcrypt.hash(password, salt);
        
        // Create user
        const user = new User({ username, email, passwordHash });
        await user.save();
        
        // Create player profile
        const profile = new PlayerProfile({ userId: user._id, displayName: username });
        await profile.save();
        
        // Create player progress (with empty arrays)
        const progress = new PlayerProgress({ userId: user._id });
        await progress.save();
        
        // Generate JWT
        const token = jwt.sign(
            { userId: user._id, username: user.username },
            JWT_SECRET,
            { expiresIn: '30d' }
        );
        
        res.status(201).json({
            message: 'User created successfully',
            token,
            user: {
                id: user._id,
                username: user.username,
                email: user.email
            }
        });
    } catch (error) {
        console.error('Registration error:', error);
        res.status(500).json({ error: 'Server error' });
    }
});

// Login
app.post('/api/auth/login', async (req, res) => {
    const { username, password } = req.body;
    
    if (!username || !password) {
        return res.status(400).json({ error: 'Username and password required' });
    }
    
    try {
        const user = await User.findOne({ username });
        if (!user) {
            return res.status(401).json({ error: 'Invalid credentials' });
        }
        
        // Check password
        const validPassword = await bcrypt.compare(password, user.passwordHash);
        if (!validPassword) {
            return res.status(401).json({ error: 'Invalid credentials' });
        }
        
        // Update last login
        user.lastLogin = new Date();
        await user.save();
        
        // Generate JWT
        const token = jwt.sign(
            { userId: user._id, username: user.username },
            JWT_SECRET,
            { expiresIn: '30d' }
        );
        
        res.json({
            message: 'Login successful',
            token,
            user: {
                id: user._id,
                username: user.username,
                email: user.email
            }
        });
    } catch (error) {
        console.error('Login error:', error);
        res.status(500).json({ error: 'Server error' });
    }
});

// Get player profile
app.get('/api/player/profile', authenticateToken, async (req, res) => {
    try {
        const profile = await PlayerProfile.findOne({ userId: req.user.userId });
        res.json(profile);
    } catch (error) {
        res.status(500).json({ error: 'Database error' });
    }
});

// Update player profile
app.put('/api/player/profile', authenticateToken, async (req, res) => {
    const { displayName, iconId, showHints, enableTutorial } = req.body;
    
    try {
        const profile = await PlayerProfile.findOneAndUpdate(
            { userId: req.user.userId },
            { displayName, iconId, showHints, enableTutorial },
            { new: true }
        );
        res.json({ message: 'Profile updated successfully', profile });
    } catch (error) {
        res.status(500).json({ error: 'Failed to update profile' });
    }
});

// Get player progress (includes inventory, unlocks, achievements)
app.get('/api/player/progress', authenticateToken, async (req, res) => {
    try {
        const progress = await PlayerProgress.findOne({ userId: req.user.userId });
        res.json(progress);
    } catch (error) {
        res.status(500).json({ error: 'Database error' });
    }
});

// Save player progress (full sync)
app.put('/api/player/progress', authenticateToken, async (req, res) => {
    const {
        level, currentXP, xpToNextLevel, skillPoints,
        totalKills, totalDeaths, totalPlayTime, totalQuestsCompleted,
        inventory, unlockedContent, achievements
    } = req.body;
    
    try {
        const updateData = {
            level, currentXP, xpToNextLevel, skillPoints,
            totalKills, totalDeaths, totalPlayTime, totalQuestsCompleted,
            lastSaved: new Date()
        };
        
        // Update arrays if provided
        if (inventory) updateData.inventory = inventory;
        if (unlockedContent) updateData.unlockedContent = unlockedContent;
        if (achievements) updateData.achievements = achievements;
        
        const progress = await PlayerProgress.findOneAndUpdate(
            { userId: req.user.userId },
            updateData,
            { new: true, upsert: true }
        );
        
        res.json({ message: 'Progress saved successfully', progress });
    } catch (error) {
        console.error('Save progress error:', error);
        res.status(500).json({ error: 'Failed to save progress' });
    }
});

// Inventory management - Add item
app.post('/api/player/inventory', authenticateToken, async (req, res) => {
    const { itemId, itemType, slot, stats } = req.body;
    
    try {
        const progress = await PlayerProgress.findOne({ userId: req.user.userId });
        progress.inventory.push({
            itemId, itemType, slot,
            isEquipped: false,
            durability: 100,
            stats: stats || {}
        });
        await progress.save();
        res.json({ message: 'Item added to inventory', inventory: progress.inventory });
    } catch (error) {
        res.status(500).json({ error: 'Failed to add item' });
    }
});

// Equip/Unequip item
app.put('/api/player/inventory/equip', authenticateToken, async (req, res) => {
    const { itemId, isEquipped } = req.body;
    
    try {
        const progress = await PlayerProgress.findOne({ userId: req.user.userId });
        const item = progress.inventory.find(item => item.itemId === itemId);
        if (item) {
            item.isEquipped = isEquipped;
            await progress.save();
            res.json({ message: `Item ${isEquipped ? 'equipped' : 'unequipped'}` });
        } else {
            res.status(404).json({ error: 'Item not found' });
        }
    } catch (error) {
        res.status(500).json({ error: 'Failed to update inventory' });
    }
});

// Unlock content
app.post('/api/player/unlock', authenticateToken, async (req, res) => {
    const { contentId, contentType } = req.body;
    
    try {
        const progress = await PlayerProgress.findOne({ userId: req.user.userId });
        
        // Check if already unlocked
        const alreadyUnlocked = progress.unlockedContent.some(item => item.contentId === contentId);
        if (alreadyUnlocked) {
            return res.status(409).json({ error: 'Content already unlocked' });
        }
        
        progress.unlockedContent.push({ contentId, contentType });
        await progress.save();
        res.json({ message: 'Content unlocked successfully' });
    } catch (error) {
        res.status(500).json({ error: 'Failed to unlock content' });
    }
});

// Update achievement
app.put('/api/player/achievement', authenticateToken, async (req, res) => {
    const { achievementId, progress: achievementProgress, isCompleted } = req.body;
    
    try {
        const progress = await PlayerProgress.findOne({ userId: req.user.userId });
        
        let achievement = progress.achievements.find(a => a.achievementId === achievementId);
        if (achievement) {
            achievement.progress = achievementProgress;
            achievement.isCompleted = isCompleted;
            if (isCompleted && !achievement.completedAt) {
                achievement.completedAt = new Date();
            }
        } else {
            progress.achievements.push({
                achievementId,
                progress: achievementProgress,
                isCompleted,
                completedAt: isCompleted ? new Date() : undefined
            });
        }
        
        await progress.save();
        res.json({ message: 'Achievement updated successfully' });
    } catch (error) {
        res.status(500).json({ error: 'Failed to update achievement' });
    }
});

// Get full player data (for sync)
app.get('/api/player/sync', authenticateToken, async (req, res) => {
    try {
        const [profile, progress] = await Promise.all([
            PlayerProfile.findOne({ userId: req.user.userId }),
            PlayerProgress.findOne({ userId: req.user.userId })
        ]);
        
        res.json({
            profile,
            progress
        });
    } catch (error) {
        res.status(500).json({ error: 'Database error' });
    }
});

// Error handling
app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).json({ error: 'Something went wrong!' });
});

// Start server
app.listen(PORT, () => {
    console.log(`Doomnite server running on port ${PORT}`);
});

module.exports = app;
