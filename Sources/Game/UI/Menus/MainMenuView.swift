// MainMenuView.swift
//  Doomnite - Main Menu with full navigation

import SwiftUI
import GameKit

struct MainMenuView: View {
    @ObservedObject var gameState: GameStateManager
    @State private var showSettings = false
    @State private var showCredits = false
    @State private var showLobby = false
    
    var body: some View {
        ZStack {
            // Background (animated)
            BackgroundView()
            
            VStack(spacing: 30) {
                Spacer()
                
                // Game Logo
                VStack(spacing: 10) {
                    Text("DOOMNITE")
                        .font(.system(size: 48, weight: .black, design: .serif))
                        .foregroundColor(.red)
                        .shadow(color: .red.opacity(0.5), radius: 10)
                    
                    Text("A Minecraft × Elder Scrolls × Destiny × Fortnite Inspired Adventure")
                        .font(.caption)
                        .foregroundColor(.gray)
                        .multilineTextAlignment(.center)
                }
                
                Spacer()
                
                // Menu Buttons
                VStack(spacing: 15) {
                    MenuButton(title: "CONTINUE GAME", icon: "play.fill") {
                        continueGame()
                    }
                    
                    MenuButton(title: "NEW GAME", icon: "plus.circle.fill") {
                        startNewGame()
                    }
                    
                    MenuButton(title: "BATTLE ROYALE", icon: "person.3.fill") {
                        showLobby = true
                    }
                    
                    MenuButton(title: "SETTINGS", icon: "gearshape.fill") {
                        showSettings = true
                    }
                    
                    MenuButton(title: "CREDITS", icon: "info.circle.fill") {
                        showCredits = true
                    }
                    
                    MenuButton(title: "EXIT", icon: "xmark.circle.fill") {
                        exitGame()
                    }
                }
                .padding(.horizontal, 40)
                
                Spacer()
                
                // Version info
                Text("v1.0.0 - Built with Metal & C++")
                    .font(.caption2)
                    .foregroundColor(.gray)
            }
            .padding(.horizontal, 30)
        }
        .sheet(isPresented: $showSettings) {
            SettingsView(gameState: gameState)
        }
        .sheet(isPresented: $showLobby) {
            LobbyView(gameState: gameState)
        }
        .sheet(isPresented: $showCredits) {
            CreditsView()
        }
    }
    
    private func continueGame() {
        // Load saved game
        gameState.loadGame()
    }
    
    private func startNewGame() {
        // Start new game
        gameState.startNewGame()
    }
    
    private func exitGame() {
        // Exit application
        exit(0)
    }
}

// MARK: - Background View

struct BackgroundView: View {
    @State private var animationAmount: CGFloat = 0
    
    var body: some View {
        ZStack {
            // Animated gradient
            LinearGradient(
                gradient: Gradient(colors: [.black, .red.opacity(0.3), .black]),
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .scaleEffect(1 + animationAmount * 0.1)
            
            // Particle overlay (would use Metal particles)
            // MetalParticleView()
        }
        .onAppear {
            withAnimation(.easeInOut(duration: 3).repeatForever(autoreverses: true)) {
                animationAmount = 1
            }
        }
    }
}

// MARK: - Menu Button

struct MenuButton: View {
    let title: String
    let icon: String
    let action: () -> Void
    
    var body: some View {
        Button(action: action) {
            HStack {
                Image(systemName: icon)
                    .frame(width: 24, height: 24)
                    .foregroundColor(.white)
                
                Text(title)
                    .font(.title3.bold())
                    .foregroundColor(.white)
                
                Spacer()
                
                Image(systemName: "chevron.right")
                    .foregroundColor(.gray)
            }
            .padding()
            .background(Color.white.opacity(0.1))
            .cornerRadius(10)
        }
        .buttonStyle(PlainButtonStyle())
    }
}

// MARK: - Settings View

struct SettingsView: View {
    @ObservedObject var gameState: GameStateManager
    @Environment(\.presentationMode) var presentationMode
    
    var body: some View {
        NavigationView {
            Form {
                // Graphics
                Section(header: Text("Graphics")) {
                    Picker("Quality", selection: $gameState.graphicsQuality) {
                        Text("Low").tag(0)
                        Text("Medium").tag(1)
                        Text("High").tag(2)
                    }
                    
                    Toggle("Thermal Throttling", isOn: $gameState.enableThermalThrottling)
                    
                    Stepper("FPS Limit: \(gameState.fpsLimit)", value: $gameState.fpsLimit, in: 30...120)
                }
                
                // Audio
                Section(header: Text("Audio")) {
                    Slider(value: $gameState.masterVolume, in: 0...1, label: {
                        Text("Master: \(Int(gameState.masterVolume * 100))%")
                    })
                    
                    Slider(value: $gameState.musicVolume, in: 0...1, label: {
                        Text("Music: \(Int(gameState.musicVolume * 100))%")
                    })
                    
                    Slider(value: $gameState.sfxVolume, in: 0...1, label: {
                        Text("SFX: \(Int(gameState.sfxVolume * 100))%")
                    })
                }
                
                // Controls
                Section(header: Text("Controls")) {
                    Toggle("Invert Y-Axis", isOn: $gameState.invertYAxis)
                    
                    Stepper("Sensitivity: \(String(format: "%.1f", gameState.sensitivity))",
                          value: $gameState.sensitivity, in: 0.1...5.0, step: 0.1)
                }
                
                // Gameplay
                Section(header: Text("Gameplay")) {
                    Toggle("Show Hints", isOn: $gameState.showHints)
                    Toggle("Enable Tutorial", isOn: $gameState.enableTutorial)
                    Toggle("Online Mode", isOn: $gameState.isOnlineMode)
                }
                
                // Reset
                Section {
                    Button("Reset to Defaults") {
                        gameState.resetToDefaults()
                    }
                    .foregroundColor(.red)
                }
            }
            .navigationBarTitle("Settings")
            .navigationBarItems(
                trailing: Button("Done") {
                    presentationMode.wrappedValue?.dismiss()
                    gameState.saveSettings()
                }
            )
        }
    }
}

// MARK: - Lobby View

struct LobbyView: View {
    @ObservedObject var gameState: GameStateManager
    @Environment(\.presentationMode) var presentationMode
    @State private var lobbyCode: String = ""
    @State private var showCreateLobby = false
    
    var body: some View {
        NavigationView {
            VStack {
                // Quick Match
                Button(action: {
                    quickMatch()
                }) {
                    HStack {
                        Image(systemName: "bolt.fill")
                        Text("QUICK MATCH")
                            .font(.title2.bold())
                    }
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.blue)
                    .cornerRadius(10)
                }
                .padding(.horizontal)
                
                // Create/Join
                HStack {
                    Button("CREATE LOBBY") {
                        showCreateLobby = true
                    }
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.green)
                    .cornerRadius(10)
                    
                    VStack {
                        TextField("Enter Code", text: $lobbyCode)
                            .textFieldStyle(RoundedBorderTextFieldStyle())
                            .multilineTextAlignment(.center)
                        
                        Button("JOIN") {
                            joinLobby()
                        }
                        .frame(maxWidth: .infinity)
                    }
                }
                .padding(.horizontal)
                
                // Active Lobbies
                List(gameState.activeLobbies) { lobby in
                    LobbyRowView(lobby: lobby)
                }
            }
            .navigationBarTitle("Battle Royale Lobby")
            .navigationBarItems(
                leading: Button("Back") {
                    presentationMode.wrappedValue?.dismiss()
                }
            )
        }
        .sheet(isPresented: $showCreateLobby) {
            CreateLobbyView(gameState: gameState)
        }
    }
    
    private func quickMatch() {
        gameState.quickMatch()
    }
    
    private func joinLobby() {
        gameState.joinLobby(code: lobbyCode)
    }
}

// MARK: - Subviews

struct LobbyRowView: View {
    let lobby: LobbyInfo
    
    var body: some View {
        HStack {
            VStack(alignment: .leading) {
                Text(lobby.name)
                    .font(.headline)
                Text("\(lobby.playerCount)/\(lobby.maxPlayers) players")
                    .font(.caption)
                    .foregroundColor(.gray)
            }
            
            Spacer()
            
            Button("JOIN") {
                // Join this lobby
            }
            .foregroundColor(.blue)
        }
    }
}

struct CreateLobbyView: View {
    @ObservedObject var gameState: GameStateManager
    @Environment(\.presentationMode) var presentationMode
    @State private var lobbyName: String = "My Lobby"
    @State private var maxPlayers: Int = 4
    @State private var isPublic: Bool = true
    
    var body: some View {
        Form {
            Section(header: Text("Lobby Settings")) {
                TextField("Lobby Name", text: $lobbyName)
                
                Stepper("Max Players: \(maxPlayers)", value: $maxPlayers, in: 2...8)
                
                Toggle("Public Lobby", isOn: $isPublic)
            }
            
            Section {
                Button("CREATE & JOIN") {
                    gameState.createLobby(name: lobbyName, maxPlayers: maxPlayers, isPublic: isPublic)
                    presentationMode.wrappedValue?.dismiss()
                }
                .frame(maxWidth: .infinity)
                .foregroundColor(.white)
                .padding()
                .background(Color.green)
                .cornerRadius(8)
            }
        }
        .navigationBarTitle("Create Lobby")
        .navigationBarItems(
            leading: Button("Cancel") {
                presentationMode.wrappedValue?.dismiss()
            }
        )
    }
}

struct CreditsView: View {
    @Environment(\.presentationMode) var presentationMode
    
    var body: some View {
        NavigationView {
            ScrollView {
                VStack(spacing: 20) {
                    Text("DOOMNITE")
                        .font(.largeTitle.bold())
                    
                    Group {
                        Text("Development Team")
                            .font(.title2.bold())
                        
                        CreditRow(title: "Lead Developer", name: "Your Name")
                        CreditRow(title: "C++ Engine", name: "Your Name")
                        CreditRow(title: "SwiftUI", name: "Your Name")
                        CreditRow(title: "Art & Design", name: "Your Name")
                    }
                    
                    Group {
                        Text("Special Thanks")
                            .font(.title2.bold())
                        
                        Text("Built with Metal, Swift, C++, and MongoDB")
                            .font(.body)
                            .multilineTextAlignment(.center)
                    }
                    
                    Group {
                        Text("Legal")
                            .font(.title2.bold())
                        
                        Text("© 2026 Your Company. All rights reserved.")
                            .font(.caption)
                            .foregroundColor(.gray)
                    }
                }
                .padding()
            }
        }
        .navigationBarTitle("Credits")
        .navigationBarItems(
            trailing: Button("Done") {
                presentationMode.wrappedValue?.dismiss()
            }
        )
    }
}

struct CreditRow: View {
    let title: String
    let name: String
    
    var body: some View {
        HStack {
            Text(title)
                .foregroundColor(.gray)
            Spacer()
            Text(name)
                .bold()
        }
    }
}

// MARK: - Game State Manager (Mock)

class GameStateManager: ObservableObject {
    @Published var graphicsQuality: Int = 2
    @Published var enableThermalThrottling: Bool = true
    @Published var fpsLimit: Int = 60
    @Published var masterVolume: Float = 1.0
    @Published var musicVolume: Float = 0.7
    @Published var sfxVolume: Float = 1.0
    @Published var invertYAxis: Bool = false
    @Published var sensitivity: Float = 1.0
    @Published var showHints: Bool = true
    @Published var enableTutorial: Bool = true
    @Published var isOnlineMode: Bool = false
    @Published var activeLobbies: [LobbyInfo] = [
        LobbyInfo(name: "Pro Player's Lobby", playerCount: 3, maxPlayers: 4, isPublic: true),
        LobbyInfo(name: "Noobs Welcome", playerCount: 2, maxPlayers: 8, isPublic: true)
    ]
    
    func loadGame() {
        // Load saved game
    }
    
    func startNewGame() {
        // Start new game
    }
    
    func saveSettings() {
        // Save to UserDefaults or server
    }
    
    func resetToDefaults() {
        graphicsQuality = 2
        enableThermalThrottling = true
        fpsLimit = 60
        masterVolume = 1.0
        musicVolume = 0.7
        sfxVolume = 1.0
        invertYAxis = false
        sensitivity = 1.0
        showHints = true
        enableTutorial = true
    }
    
    func quickMatch() {
        // Quick match
    }
    
    func joinLobby(code: String) {
        // Join lobby with code
    }
    
    func createLobby(name: String, maxPlayers: Int, isPublic: Bool) {
        // Create new lobby
    }
}

struct LobbyInfo {
    let name: String
    let playerCount: Int
    let maxPlayers: Int
    let isPublic: Bool
}

// MARK: - Preview

struct MainMenuView_Previews: PreviewProvider {
    static var previews: some View {
        MainMenuView(gameState: GameStateManager())
    }
}
