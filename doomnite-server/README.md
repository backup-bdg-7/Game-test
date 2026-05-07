# Doomnite Server

Node.js/Express server for the Doomnite iOS game. Handles:
- User authentication (register/login)
- Player progress synchronization between devices
- Achievement tracking
- Unlocked content management

## Features

- **JWT Authentication** - Secure token-based auth
- **SQLite Database** - Lightweight, serverless database
- **Player Progress Sync** - Save/load game state across devices
- **Content Unlocking** - Track weapons, armor, spells, companions
- **Achievement System** - Track and award achievements
- **CORS Support** - Ready for iOS app integration

## Prerequisites

- Node.js 18+ 
- npm or yarn

## Installation

1. Clone the repository:
```bash
git clone <repo-url>
cd doomnite-server
```

2. Install dependencies:
```bash
npm install
```

3. Create `.env` file from example:
```bash
cp config/.env.example .env
# Edit .env and set your JWT_SECRET!
```

4. Start the server:
```bash
npm start
# or for development with auto-reload:
npm run dev
```

Server will start on `http://localhost:3000` (or PORT env variable).

## API Endpoints

### Authentication

#### Register
```http
POST /api/auth/register
Content-Type: application/json

{
  "username": "PlayerOne",
  "email": "player@example.com",
  "password": "securepassword"
}
```

#### Login
```http
POST /api/auth/login
Content-Type: application/json

{
  "username": "PlayerOne",
  "password": "securepassword"
}
```

### Player Data (Requires Auth Token)

#### Get Profile
```http
GET /api/player/profile
Authorization: Bearer <your-jwt-token>
```

#### Update Profile
```http
PUT /api/player/profile
Authorization: Bearer <your-jwt-token>
Content-Type: application/json

{
  "display_name": "Player One",
  "icon_id": "default_2",
  "show_hints": true,
  "enable_tutorial": true
}
```

#### Get Progress
```http
GET /api/player/progress
Authorization: Bearer <your-jwt-token>
```

#### Save Progress
```http
PUT /api/player/progress
Authorization: Bearer <your-jwt-token>
Content-Type: application/json

{
  "level": 15,
  "current_xp": 2500,
  "xp_to_next_level": 5000,
  "skill_points": 30,
  "total_kills": 450,
  "total_deaths": 12,
  "total_play_time": 3600.5,
  "total_quests_completed": 25
}
```

#### Get Unlocked Content
```http
GET /api/player/unlocked
Authorization: Bearer <your-jwt-token>
```

#### Unlock Content
```http
POST /api/player/unlocked
Authorization: Bearer <your-jwt-token>
Content-Type: application/json

{
  "content_id": "weapon_staff_fire",
  "content_type": "Weapon"
}
```

#### Get Achievements
```http
GET /api/player/achievements
Authorization: Bearer <your-jwt-token>
```

#### Update Achievement
```http
PUT /api/player/achievements
Authorization: Bearer <your-jwt-token>
Content-Type: application/json

{
  "achievement_id": "first_blood",
  "progress": 1.0,
  "is_completed": true
}
```

#### Full Sync (Get all player data at once)
```http
GET /api/player/sync
Authorization: Bearer <your-jwt-token>
```

## Database Schema

### Tables
- **users** - Authentication data (username, email, password hash)
- **player_profiles** - Display name, icon, settings
- **player_progress** - Level, XP, stats
- **unlocked_content** - Weapons, armor, spells unlocked
- **achievements** - Achievement progress

## Connecting from iOS App

In your iOS app (offline/online mode toggle):

```swift
// Example: Save progress to server
func saveProgressToServer() {
    guard let token = UserDefaults.standard.string(forKey: "authToken") else { return }
    
    var request = URLRequest(url: URL(string: "https://your-server.com/api/player/progress")!)
    request.httpMethod = "PUT"
    request.setValue("Bearer \(token)", forHTTPHeaderField: "Authorization")
    request.setValue("application/json", forHTTPHeaderField: "Content-Type")
    
    let progressData = [
        "level": player.level,
        "current_xp": player.xp,
        // ... other fields
    ] as [String: Any]
    
    request.httpBody = try? JSONSerialization.data(withJSONObject: progressData)
    
    URLSession.shared.dataTask(with: request) { data, response, error in
        // Handle response
    }.resume()
}
```

## Production Deployment

1. Set strong `JWT_SECRET` in environment
2. Use HTTPS (SSL/TLS)
3. Consider using PostgreSQL instead of SQLite for scalability
4. Add rate limiting
5. Set up database backups

## License

MIT
