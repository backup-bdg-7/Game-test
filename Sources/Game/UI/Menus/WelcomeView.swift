//  WelcomeView.swift
//  Doomnite - Welcome/Setup Screen for iOS
//  Handles username creation, icon selection, and game settings

import SwiftUI
import UIKit

// MARK: - Welcome View (First Launch)

struct WelcomeView: View {
    @State private var username: String = ""
    @State private var selectedIcon: PlayerIcon = .defaultIcons.first!
    @State private var useCustomPhoto: Bool = false
    @State private var showImagePicker: Bool = false
    @State private var customImage: UIImage?
    @State private var showHints: Bool = true
    @State private var enableTutorial: Bool = true
    @State private var isOnlineMode: Bool = false
    @State private var serverUrl: String = "https://doomnite-server.com"
    @State private var currentStep: SetupStep = .welcome
    @State private var isSaving: Bool = false
    
    enum SetupStep {
        case welcome
        case username
        case iconSelection
        case settings
        case complete
    }
    
    var body: some View {
        ZStack {
            // Background
            Color.black.ignoresSafeArea()
            
            // Animated background particles (would use Metal particles)
            // BackgroundParticlesView()
            
            VStack(spacing: 30) {
                switch currentStep {
                case .welcome:
                    welcomeSection
                case .username:
                    usernameSection
                case .iconSelection:
                    iconSelectionSection
                case .settings:
                    settingsSection
                case .complete:
                    completeSection
                }
            }
            .padding(.horizontal, 30)
        }
        .sheet(isPresented: $showImagePicker) {
            ImagePicker(image: $customImage)
        }
    }
    
    // MARK: - Welcome Section
    
    var welcomeSection: some View {
        VStack(spacing: 20) {
            Spacer()
            
            // Game Logo
            Text("DOOMNITE")
                .font(.system(size: 48, weight: .black, design: .serif))
                .foregroundColor(.red)
                .shadow(color: .red.opacity(0.5), radius: 10)
            
            Text("A Minecraft x Elder Scrolls x Destiny x Fortnite x Animal Jam Inspired Adventure")
                .font(.caption)
                .foregroundColor(.gray)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 40)
            
            Spacer()
            
            Button(action: {
                withAnimation {
                    currentStep = .username
                }
            }) {
                Text("BEGIN JOURNEY")
                    .font(.title3.bold())
                    .foregroundColor(.white)
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.red)
                    .cornerRadius(10)
            }
            
            Button(action: {
                // Load existing game
            }) {
                Text("CONTINUE")
                    .font(.callout)
                    .foregroundColor(.gray)
            }
        }
    }
    
    // MARK: - Username Section
    
    var usernameSection: some View {
        VStack(spacing: 20) {
            Text("CHOOSE YOUR NAME")
                .font(.title2.bold())
                .foregroundColor(.white)
            
            Text("This will be your identity in the world of Doomnite")
                .font(.caption)
                .foregroundColor(.gray)
                .multilineTextAlignment(.center)
            
            TextField("Enter username...", text: $username)
                .textFieldStyle(RoundedBorderTextFieldStyle())
                .padding(.horizontal)
                .autocapitalization(.none)
            
            if username.isEmpty {
                Text("Or use a random name:")
                    .font(.caption)
                    .foregroundColor(.gray)
                
                Button(action: {
                    username = PlayerIcon.generateRandomUsername()
                }) {
                    Text("GENERATE RANDOM NAME")
                        .font(.callout.bold())
                        .foregroundColor(.white)
                        .padding(.horizontal, 20)
                        .padding(.vertical, 10)
                        .background(Color.blue)
                        .cornerRadius(8)
                }
            }
            
            Spacer()
            
            HStack {
                Button("Back") {
                    withAnimation {
                        currentStep = .welcome
                    }
                }
                .foregroundColor(.gray)
                
                Spacer()
                
                Button("Next") {
                    withAnimation {
                        currentStep = .iconSelection
                    }
                }
                .disabled(username.isEmpty)
                .foregroundColor(username.isEmpty ? .gray : .white)
                .padding(.horizontal, 20)
                .padding(.vertical, 10)
                .background(username.isEmpty ? Color.gray : Color.red)
                .cornerRadius(8)
            }
        }
    }
    
    // MARK: - Icon Selection Section
    
    var iconSelectionSection: some View {
        VStack(spacing: 20) {
            Text("CHOOSE YOUR ICON")
                .font(.title2.bold())
                .foregroundColor(.white)
            
            Text("Select a default icon or use your own photo")
                .font(.caption)
                .foregroundColor(.gray)
                .multilineTextAlignment(.center")
            
            // Custom photo option
            if useCustomPhoto {
                if let image = customImage {
                    Image(uiImage: image)
                        .resizable()
                        .scaledToFill()
                        .frame(width: 100, height: 100)
                        .clipShape(Circle())
                        .overlay(Circle().stroke(Color.red, lineWidth: 3))
                }
                
                Button("Change Photo") {
                    showImagePicker = true
                }
                .foregroundColor(.blue)
            } else {
                // Default icons grid
                LazyVGrid(columns: Array(repeating: GridItem(.flexible()), count: 4), spacing: 15) {
                    ForEach(PlayerIcon.defaultIcons) { icon in
                        PlayerIconView(icon: icon, isSelected: icon.id == selectedIcon.id)
                            .onTapGesture {
                                selectedIcon = icon
                            }
                    }
                }
                .padding(.horizontal)
            }
            
            Toggle("Use Custom Photo", isOn: $useCustomPhoto)
                .foregroundColor(.white)
                .padding(.horizontal)
            
            Spacer()
            
            HStack {
                Button("Back") {
                    withAnimation {
                        currentStep = .username
                    }
                }
                .foregroundColor(.gray)
                
                Spacer()
                
                Button("Next") {
                    withAnimation {
                        currentStep = .settings
                    }
                }
                .foregroundColor(.white)
                .padding(.horizontal, 20)
                .padding(.vertical, 10)
                .background(Color.red)
                .cornerRadius(8)
            }
        }
    }
    
    // MARK: - Settings Section
    
    var settingsSection: some View {
        VStack(spacing: 20) {
            Text("GAME SETTINGS")
                .font(.title2.bold())
                .foregroundColor(.white)
            
            // Hints toggle
            SettingToggleView(
                title: "Show Hints",
                description: "Display helpful tips and explanations during gameplay",
                isOn: $showHints
            )
            
            // Tutorial toggle
            SettingToggleView(
                title: "Enable Tutorial",
                description: "Guide you through game mechanics when first encountered",
                isOn: $enableTutorial
            )
            
            Divider()
                .background(Color.gray)
            
            // Online mode toggle
            SettingToggleView(
                title: "Online Mode",
                description: "Sync progress to server (requires internet)",
                isOn: $isOnlineMode
            )
            
            if isOnlineMode {
                TextField("Server URL", text: $serverUrl)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .padding(.horizontal)
                    .autocapitalization(.none)
                    .keyboardType(.URL)
            }
            
            Spacer()
            
            HStack {
                Button("Back") {
                    withAnimation {
                        currentStep = .iconSelection
                    }
                }
                .foregroundColor(.gray)
                
                Spacer()
                
                if isSaving {
                    ProgressView()
                } else {
                    Button("START GAME") {
                        savePlayerData()
                    }
                    .foregroundColor(.white)
                    .padding(.horizontal, 20)
                    .padding(.vertical, 10)
                    .background(Color.red)
                    .cornerRadius(8)
                }
            }
        }
    }
    
    // MARK: - Complete Section
    
    var completeSection: some View {
        VStack(spacing: 20) {
            Spacer()
            
            Image(systemName: "checkmark.circle.fill")
                .resizable()
                .scaledToFit()
                .frame(width: 80, height: 80)
                .foregroundColor(.green)
            
            Text("READY TO PLAY!")
                .font(.title.bold())
                .foregroundColor(.white)
            
            Text("Welcome to Doomnite, \(username)!")
                .font(.headline)
                .foregroundColor(.gray)
            
            Spacer()
            
            Button("ENTER WORLD") {
                // Transition to main game
            }
            .font(.title3.bold())
            .foregroundColor(.white)
            .frame(maxWidth: .infinity)
            .padding()
            .background(Color.red)
            .cornerRadius(10)
        }
    }
    
    // MARK: - Helper Methods
    
    func savePlayerData() {
        isSaving = true
        
        // Create player data
        let playerData = PlayerData(
            username: username,
            iconId: selectedIcon.id,
            useCustomPhoto: useCustomPhoto,
            customPhotoPath: "",  // Would save image to documents
            showHints: showHints,
            enableTutorial: enableTutorial,
            isOnlineMode: isOnlineMode,
            serverUrl: serverUrl
        )
        
        // Save locally (or to server if online mode)
        if isOnlineMode {
            // Would sync to server
            // syncToServer(playerData)
        } else {
            saveLocally(playerData)
        }
        
        // Save custom photo if used
        if useCustomPhoto, let image = customImage {
            saveCustomPhoto(image)
        }
        
        isSaving = false
        
        withAnimation {
            currentStep = .complete
        }
    }
    
    func saveLocally(_ playerData: PlayerData) {
        // Save to UserDefaults or file
        // In production, would use Core Data or file serialization
    }
    
    func saveCustomPhoto(_ image: UIImage) {
        // Save to Documents directory
        // Would use FileManager to save the image
    }
}

// MARK: - Player Icon Model

struct PlayerIcon: Identifiable {
    let id: String
    let name: String
    let imageName: String?  // nil for custom
    let color: Color
    
    static let defaultIcons: [PlayerIcon] = [
        PlayerIcon(id: "default_1", name: "Warrior", imageName: "person.fill", color: .blue),
        PlayerIcon(id: "default_2", name: "Mage", imageName: "wand.and.stars", color: .purple),
        PlayerIcon(id: "default_3", name: "Archer", imageName: "scope", color: .green),
        PlayerIcon(id: "default_4", name: "Dragon", imageName: "flame.fill", color: .red),
        PlayerIcon(id: "default_5", name: "Wolf", imageName: "pawprint.fill", color: .gray),
        PlayerIcon(id: "default_6", name: "Bear", imageName: "bear.fill", color: .brown),
        PlayerIcon(id: "default_7", name: "Eagle", imageName: "bird.fill", color: .orange),
        PlayerIcon(id: "default_8", name: "Knight", imageName: "shield.fill", color: .cyan),
    ]
    
    static func generateRandomUsername() -> String {
        let prefixes = ["Brave", "Mighty", "Epic", "Noble", "Shadow", "Fire", "Ice", "Storm"]
        let suffixes = ["Warrior", "Mage", "Hunter", "Knight", "Dragon", "Wolf", "Bear", "Eagle"]
        
        let prefix = prefixes.randomElement()!
        let suffix = suffixes.randomElement()!
        let number = Int.random(in: 1...999)
        
        return "\(prefix)\(suffix)\(number)"
    }
}

// MARK: - Subviews

struct PlayerIconView: View {
    let icon: PlayerIcon
    let isSelected: Bool
    
    var body: some View {
        VStack {
            if let imageName = icon.imageName {
                Image(systemName: imageName)
                    .resizable()
                    .scaledToFit()
                    .frame(width: 40, height: 40)
                    .foregroundColor(icon.color)
            } else {
                Image(systemName: "person.crop.circle")
                    .resizable()
                    .scaledToFit()
                    .frame(width: 40, height: 40)
                    .foregroundColor(.gray)
            }
        }
        .padding(10)
        .background(isSelected ? Color.white.opacity(0.2) : Color.clear)
        .cornerRadius(8)
        .overlay(
            RoundedRectangle(cornerRadius: 8)
                .stroke(isSelected ? Color.red : Color.clear, lineWidth: 2)
        )
    }
}

struct SettingToggleView: View {
    let title: String
    let description: String
    @Binding var isOn: Bool
    
    var body: some View {
        VStack(alignment: .leading, spacing: 5) {
            Toggle(title, isOn: $isOn)
                .foregroundColor(.white)
            
            Text(description)
                .font(.caption)
                .foregroundColor(.gray)
        }
        .padding(.horizontal)
    }
}

// MARK: - Image Picker

struct ImagePicker: UIViewControllerRepresentable {
    @Binding var image: UIImage?
    @Environment(\.presentationMode) var presentationMode
    
    func makeUIViewController(context: Context) -> UIImagePickerController {
        let picker = UIImagePickerController()
        picker.delegate = context.coordinator
        picker.sourceType = .photoLibrary
        return picker
    }
    
    func updateUIViewController(_ uiViewController: UIImagePickerController, context: Context) {}
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    class Coordinator: NSObject, UIImagePickerControllerDelegate, UINavigationControllerDelegate {
        let parent: ImagePicker
        
        init(_ parent: ImagePicker) {
            self.parent = parent
        }
        
        func imagePickerController(_ picker: UIImagePickerController, didFinishPickingMediaWithInfo info: [UIImagePickerController.InfoKey : Any]) {
            if let image = info[.originalImage] as? UIImage {
                parent.image = image
            }
            parent.presentationMode.wrappedValue.dismiss()
        }
        
        func imagePickerControllerDidCancel(_ picker: UIImagePickerController) {
            parent.presentationMode.wrappedValue.dismiss()
        }
    }
}

// MARK: - Player Data Model

struct PlayerData: Codable {
    let username: String
    let iconId: String
    let useCustomPhoto: Bool
    let customPhotoPath: String
    let showHints: Bool
    let enableTutorial: Bool
    let isOnlineMode: Bool
    let serverUrl: String
}

// MARK: - Preview

struct WelcomeView_Previews: PreviewProvider {
    static var previews: some View {
        WelcomeView()
    }
}
