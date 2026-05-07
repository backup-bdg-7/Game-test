// SIMD Math Library for Doomnite
// Provides vector/matrix math for 3D game engine

import Foundation
import simd

// MARK: - Vector Types (Type Aliases)

public typealias Vec2 = SIMD2<Float>
public typealias Vec3 = SIMD3<Float>
public typealias Vec4 = SIMD4<Float>
public typealias Mat4 = simd_float4x4
public typealias Quat = simd_quatf

// MARK: - Vector3 Extensions

extension Vec3 {
    // Common vectors
    public static let zero = Vec3(0, 0, 0)
    public static let one = Vec3(1, 1, 1)
    public static let up = Vec3(0, 1, 0)
    public static let down = Vec3(0, -1, 0)
    public static let left = Vec3(-1, 0, 0)
    public static let right = Vec3(1, 0, 0)
    public static let forward = Vec3(0, 0, 1)
    public static let back = Vec3(0, 0, -1)
    
    // Computed properties
    public var x: Float { return self[0] }
    public var y: Float { return self[1] }
    public var z: Float { return self[2] }
    
    public var magnitude: Float {
        return simd_length(self)
    }
    
    public var magnitudeSquared: Float {
        return simd_length_squared(self)
    }
    
    public var normalized: Vec3 {
        return simd_normalize(self)
    }
    
    // Component-wise operations
    public func cross(_ other: Vec3) -> Vec3 {
        return simd_cross(self, other)
    }
    
    public func dot(_ other: Vec3) -> Float {
        return simd_dot(self, other)
    }
    
    public func distance(to other: Vec3) -> Float {
        return simd_distance(self, other)
    }
    
    // 2D projections
    public var xy: Vec2 {
        return Vec2(x, y)
    }
    
    public var xz: Vec2 {
        return Vec2(x, z)
    }
    
    public var yz: Vec2 {
        return Vec2(y, z)
    }
    
    // Lerp
    public static func lerp(from: Vec3, to: Vec3, t: Float) -> Vec3 {
        return simd_mix(from, to, t: t)
    }
}

// MARK: - Matrix4x4 Extensions

extension Mat4 {
    public static let identity = matrix_identity_float4x4
    
    // Transformation matrices
    public static func translation(_ vector: Vec3) -> Mat4 {
        var matrix = Mat4.identity
        matrix.columns.3 = Vec4(vector.x, vector.y, vector.z, 1.0)
        return matrix
    }
    
    public static func scaling(_ factor: Float) -> Mat4 {
        return Mat4(diagonal: Vec4(factor, factor, factor, 1.0))
    }
    
    public static func scaling(_ vector: Vec3) -> Mat4 {
        return Mat4(diagonal: Vec4(vector.x, vector.y, vector.z, 1.0))
    }
    
    public static func rotationX(_ angle: Float) -> Mat4 {
        let cosA = cos(angle)
        let sinA = sin(angle)
        return Mat4(
            Vec4(1, 0, 0, 0),
            Vec4(0, cosA, sinA, 0),
            Vec4(0, -sinA, cosA, 0),
            Vec4(0, 0, 0, 1)
        )
    }
    
    public static func rotationY(_ angle: Float) -> Mat4 {
        let cosA = cos(angle)
        let sinA = sin(angle)
        return Mat4(
            Vec4(cosA, 0, -sinA, 0),
            Vec4(0, 1, 0, 0),
            Vec4(sinA, 0, cosA, 0),
            Vec4(0, 0, 0, 1)
        )
    }
    
    public static func rotationZ(_ angle: Float) -> Mat4 {
        let cosA = cos(angle)
        let sinA = sin(angle)
        return Mat4(
            Vec4(cosA, sinA, 0, 0),
            Vec4(-sinA, cosA, 0, 0),
            Vec4(0, 0, 1, 0),
            Vec4(0, 0, 0, 1)
        )
    }
    
    public static func rotationEuler(pitch: Float, yaw: Float, roll: Float) -> Mat4 {
        return rotationZ(roll) * rotationX(pitch) * rotationY(yaw)
    }
    
    public static func rotation(quaternion: Quat) -> Mat4 {
        return simd_matrix_from_quaternion(quaternion)
    }
    
    // Camera matrices
    public static func lookAt(eye: Vec3, center: Vec3, up: Vec3) -> Mat4 {
        let z = simd_normalize(eye - center)
        let x = simd_normalize(simd_cross(up, z))
        let y = simd_cross(z, x)
        
        var matrix = Mat4.identity
        matrix.columns.0 = Vec4(x.x, y.x, z.x, 0)
        matrix.columns.1 = Vec4(x.y, y.y, z.y, 0)
        matrix.columns.2 = Vec4(x.z, y.z, z.z, 0)
        matrix.columns.3 = Vec4(-simd_dot(x, eye), -simd_dot(y, eye), -simd_dot(z, eye), 1)
        return matrix
    }
    
    public static func perspective(fovY: Float, aspect: Float, near: Float, far: Float) -> Mat4 {
        let f = 1.0 / tan(fovY * 0.5)
        let nf = 1.0 / (near - far)
        
        var matrix = Mat4()
        matrix.columns.0 = Vec4(f / aspect, 0, 0, 0)
        matrix.columns.1 = Vec4(0, f, 0, 0)
        matrix.columns.2 = Vec4(0, 0, (far + near) * nf, -1)
        matrix.columns.3 = Vec4(0, 0, 2 * far * near * nf, 0)
        return matrix
    }
    
    public static func orthographic(left: Float, right: Float, bottom: Float, top: Float, near: Float, far: Float) -> Mat4 {
        var matrix = Mat4()
        matrix.columns.0 = Vec4(2 / (right - left), 0, 0, 0)
        matrix.columns.1 = Vec4(0, 2 / (top - bottom), 0, 0)
        matrix.columns.2 = Vec4(0, 0, -2 / (far - near), 0)
        matrix.columns.3 = Vec4(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1)
        return matrix
    }
}

// MARK: - Quaternion Extensions

extension Quat {
    public static let identity = Quat(ix: 0, iy: 0, iz: 0, r: 1)
    
    public static func fromEuler(pitch: Float, yaw: Float, roll: Float) -> Quat {
        let qx = Quat(angle: pitch, axis: Vec3(1, 0, 0))
        let qy = Quat(angle: yaw, axis: Vec3(0, 1, 0))
        let qz = Quat(angle: roll, axis: Vec3(0, 0, 1))
        return qz * qy * qx
    }
    
    public func toEuler() -> Vec3 {
        // Pitch (x-axis rotation)
        let sinPitch = 2 * (r * ix - iy * iz)
        let pitch: Float
        if abs(sinPitch) >= 1 {
            pitch = copysign(Float.pi / 2, sinPitch) // Clamp to ±π/2
        } else {
            pitch = asin(sinPitch)
        }
        
        // Yaw (y-axis rotation) and Roll (z-axis rotation)
        let sinYaw = 2 * (r * iy + ix * iz)
        let cosYaw = 1 - 2 * (ix * ix + iy * iy)
        let yaw = atan2(sinYaw, cosYaw)
        
        let sinRoll = 2 * (r * iz + ix * iy)
        let cosRoll = 1 - 2 * (ix * ix + iz * iz)
        let roll = atan2(sinRoll, cosRoll)
        
        return Vec3(pitch, yaw, roll)
    }
}

// MARK: - Math Utilities

public struct MathUtils {
    public static let epsilon: Float = 0.0001
    public static let pi: Float = Float.pi
    public static let twoPi: Float = Float.pi * 2
    public static let halfPi: Float = Float.pi / 2
    
    public static func degreesToRadians(_ degrees: Float) -> Float {
        return degrees * pi / 180.0
    }
    
    public static func radiansToDegrees(_ radians: Float) -> Float {
        return radians * 180.0 / pi
    }
    
    public static func clamp<T: Comparable>(_ value: T, min: T, max: T) -> T {
        return Swift.min(Swift.max(value, min), max)
    }
    
    public static func lerp<T: FloatingPointArithmetic>(from: T, to: T, t: T) -> T {
        return from + (to - from) * t
    }
    
    public static func inverseLerp<T: FloatingPointArithmetic>(value: T, min: T, max: T) -> T {
        return (value - min) / (max - min)
    }
    
    public static func remap(value: Float, inMin: Float, inMax: Float, outMin: Float, outMax: Float) -> Float {
        let t = inverseLerp(value: value, min: inMin, max: inMax)
        return lerp(from: outMin, to: outMax, t: t)
    }
    
    public static func smoothstep(t: Float) -> Float {
        return t * t * (3 - 2 * t)
    }
    
    public static func smootherstep(t: Float) -> Float {
        return t * t * t * (t * (t * 6 - 15) + 10)
    }
}

// MARK: - Floating Point Arithmetic Protocol

public protocol FloatingPointArithmetic: FloatingPoint {
    static func +(lhs: Self, rhs: Self) -> Self
    static func -(lhs: Self, rhs: Self) -> Self
    static func *(lhs: Self, rhs: Self) -> Self
}

extension Float: FloatingPointArithmetic {}
extension Double: FloatingPointArithmetic {}
