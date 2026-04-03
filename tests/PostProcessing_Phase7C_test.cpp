#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <string>

#define DJROOFRAT_OPENGL_MIGRATION 1
#define DJROOFRAT_ENABLE_GRAPHICS 1

// Prevent Windows from including its own OpenGL headers
#define __gl_h_
#define __GL_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Helper: Load file to string
static std::string LoadFile(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return "";
    std::string contents;
    file.seekg(0, std::ios::end);
    contents.resize(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);
    file.read(&contents[0], contents.size());
    file.close();
    return contents;
}

// Helper: Compile vertex shader
static GLuint CompileVertexShader(const char* src, std::string& error) {
    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> log(logLength);
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            error = log.data();
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// Helper: Compile fragment shader
static GLuint CompileFragmentShader(const char* src, std::string& error) {
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> log(logLength);
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            error = log.data();
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// Helper: Link post-processing program (vertex + fragment)
static GLuint LinkPostProcessProgram(GLuint vertShader, GLuint fragShader, std::string& error) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertShader);
    glAttachShader(prog, fragShader);
    glLinkProgram(prog);
    GLint status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status) {
        GLint logLength = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> log(logLength);
            glGetProgramInfoLog(prog, logLength, nullptr, log.data());
            error = log.data();
        }
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

int main() {
    std::cout << "========== Post-Processing GLSL Shader Phase 7C Tests ==========" << std::endl << std::endl;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(640, 480, "Post-Processing Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    int testsPassed = 0;
    int totalTests = 6;
    
    // Test 1: Compile postprocess.vert shader
    std::cout << "Test 1: Compile postprocess.vert shader..." << std::endl;
    {
        std::string shaderSrc = LoadFile("shaders/postprocess.vert");
        if (shaderSrc.empty()) {
            std::cout << "  FAILED: Could not load shaders/postprocess.vert" << std::endl;
        } else {
            std::string error;
            GLuint shader = CompileVertexShader(shaderSrc.c_str(), error);
            if (shader == 0) {
                std::cout << "  FAILED: Shader compilation failed: " << error << std::endl;
            } else {
                std::cout << "  PASSED: Vertex shader compiled successfully" << std::endl;
                glDeleteShader(shader);
                testsPassed++;
            }
        }
    }
    
    // Test 2: Compile bloom.frag shader
    std::cout << "Test 2: Compile bloom.frag shader..." << std::endl;
    {
        std::string shaderSrc = LoadFile("shaders/bloom.frag");
        if (shaderSrc.empty()) {
            std::cout << "  FAILED: Could not load shaders/bloom.frag" << std::endl;
        } else {
            std::string error;
            GLuint shader = CompileFragmentShader(shaderSrc.c_str(), error);
            if (shader == 0) {
                std::cout << "  FAILED: Shader compilation failed: " << error << std::endl;
            } else {
                std::cout << "  PASSED: Bloom fragment shader compiled successfully" << std::endl;
                glDeleteShader(shader);
                testsPassed++;
            }
        }
    }
    
    // Test 3: Compile colorgrade.frag shader
    std::cout << "Test 3: Compile colorgrade.frag shader..." << std::endl;
    {
        std::string shaderSrc = LoadFile("shaders/colorgrade.frag");
        if (shaderSrc.empty()) {
            std::cout << "  FAILED: Could not load shaders/colorgrade.frag" << std::endl;
        } else {
            std::string error;
            GLuint shader = CompileFragmentShader(shaderSrc.c_str(), error);
            if (shader == 0) {
                std::cout << "  FAILED: Shader compilation failed: " << error << std::endl;
            } else {
                std::cout << "  PASSED: Color grade fragment shader compiled successfully" << std::endl;
                glDeleteShader(shader);
                testsPassed++;
            }
        }
    }
    
    // Test 4: Link bloom program (postprocess.vert + bloom.frag) and create framebuffer
    std::cout << "Test 4: Link bloom program and create framebuffer..." << std::endl;
    {
        std::string vertSrc = LoadFile("shaders/postprocess.vert");
        std::string fragSrc = LoadFile("shaders/bloom.frag");
        
        if (vertSrc.empty() || fragSrc.empty()) {
            std::cout << "  FAILED: Could not load shaders" << std::endl;
        } else {
            std::string error;
            GLuint vertShader = CompileVertexShader(vertSrc.c_str(), error);
            if (vertShader == 0) {
                std::cout << "  FAILED: Vertex shader compilation failed: " << error << std::endl;
            } else {
                GLuint fragShader = CompileFragmentShader(fragSrc.c_str(), error);
                if (fragShader == 0) {
                    std::cout << "  FAILED: Fragment shader compilation failed: " << error << std::endl;
                    glDeleteShader(vertShader);
                } else {
                    GLuint program = LinkPostProcessProgram(vertShader, fragShader, error);
                    glDeleteShader(vertShader);
                    glDeleteShader(fragShader);
                    
                    if (program == 0) {
                        std::cout << "  FAILED: Program linking failed: " << error << std::endl;
                    } else {
                        // Create framebuffer
                        GLuint fbo;
                        glGenFramebuffers(1, &fbo);
                        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                        
                        // Create color texture
                        GLuint colorTex;
                        glGenTextures(1, &colorTex);
                        glBindTexture(GL_TEXTURE_2D, colorTex);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 256, 256, 0, GL_RGBA, GL_FLOAT, nullptr);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        
                        // Attach color texture to framebuffer
                        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
                        
                        // Check framebuffer status
                        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
                            std::cout << "  FAILED: Framebuffer incomplete (status: 0x" << std::hex << fboStatus << std::dec << ")" << std::endl;
                        } else {
                            std::cout << "  PASSED: Bloom program linked and framebuffer created successfully" << std::endl;
                            testsPassed++;
                        }
                        
                        glDeleteTextures(1, &colorTex);
                        glDeleteFramebuffers(1, &fbo);
                        glDeleteProgram(program);
                    }
                }
            }
        }
    }
    
    // Test 5: Link color grade program and render to framebuffer
    std::cout << "Test 5: Link color grade program and render to framebuffer..." << std::endl;
    {
        std::string vertSrc = LoadFile("shaders/postprocess.vert");
        std::string fragSrc = LoadFile("shaders/colorgrade.frag");
        
        if (vertSrc.empty() || fragSrc.empty()) {
            std::cout << "  FAILED: Could not load shaders" << std::endl;
        } else {
            std::string error;
            GLuint vertShader = CompileVertexShader(vertSrc.c_str(), error);
            if (vertShader == 0) {
                std::cout << "  FAILED: Vertex shader compilation failed: " << error << std::endl;
            } else {
                GLuint fragShader = CompileFragmentShader(fragSrc.c_str(), error);
                if (fragShader == 0) {
                    std::cout << "  FAILED: Fragment shader compilation failed: " << error << std::endl;
                    glDeleteShader(vertShader);
                } else {
                    GLuint program = LinkPostProcessProgram(vertShader, fragShader, error);
                    glDeleteShader(vertShader);
                    glDeleteShader(fragShader);
                    
                    if (program == 0) {
                        std::cout << "  FAILED: Program linking failed: " << error << std::endl;
                    } else {
                        // Create framebuffer and texture
                        GLuint fbo, colorTex;
                        glGenFramebuffers(1, &fbo);
                        glGenTextures(1, &colorTex);
                        
                        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                        glBindTexture(GL_TEXTURE_2D, colorTex);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
                        
                        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
                            std::cout << "  FAILED: Framebuffer incomplete" << std::endl;
                        } else {
                            // Create fullscreen quad VAO/VBO
                            struct Vertex {
                                glm::vec2 position;
                                glm::vec2 texCoord;
                            };
                            
                            Vertex vertices[4] = {
                                {{-1.0f, -1.0f}, {0.0f, 0.0f}},
                                {{1.0f, -1.0f}, {1.0f, 0.0f}},
                                {{1.0f, 1.0f}, {1.0f, 1.0f}},
                                {{-1.0f, 1.0f}, {0.0f, 1.0f}}
                            };
                            
                            uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
                            
                            GLuint vao, vbo, ebo;
                            glGenVertexArrays(1, &vao);
                            glGenBuffers(1, &vbo);
                            glGenBuffers(1, &ebo);
                            
                            glBindVertexArray(vao);
                            glBindBuffer(GL_ARRAY_BUFFER, vbo);
                            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                            
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
                            
                            glEnableVertexAttribArray(0);
                            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
                            
                            glEnableVertexAttribArray(1);
                            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec2)));
                            
                            // Set viewport and render
                            glViewport(0, 0, 256, 256);
                            glUseProgram(program);
                            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                            glClear(GL_COLOR_BUFFER_BIT);
                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
                            
                            std::cout << "  PASSED: Color grade program linked and rendered successfully" << std::endl;
                            testsPassed++;
                            
                            glDeleteBuffers(1, &vbo);
                            glDeleteBuffers(1, &ebo);
                            glDeleteVertexArrays(1, &vao);
                        }
                        
                        glDeleteTextures(1, &colorTex);
                        glDeleteFramebuffers(1, &fbo);
                        glDeleteProgram(program);
                    }
                }
            }
        }
    }
    
    // Test 6: Verify framebuffer color output (sanity check)
    std::cout << "Test 6: Verify framebuffer color output..." << std::endl;
    {
        GLuint fbo, colorTex;
        glGenFramebuffers(1, &fbo);
        glGenTextures(1, &colorTex);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
        
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "  FAILED: Framebuffer incomplete" << std::endl;
        } else {
            // Clear framebuffer to known color
            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            // Read back pixel data
            uint8_t pixelData[4 * 4 * 4];  // 4x4 RGBA8
            glReadPixels(0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
            
            // Verify at least some pixels are approximately (128, 128, 128, 255)
            // 0.5f * 255 ≈ 127-128
            bool pixelsValid = false;
            for (int i = 0; i < 4 * 4 * 4; i += 4) {
                uint8_t r = pixelData[i];
                uint8_t g = pixelData[i + 1];
                uint8_t b = pixelData[i + 2];
                uint8_t a = pixelData[i + 3];
                
                // Allow tolerance for rounding
                if ((r >= 120 && r <= 135) && (g >= 120 && g <= 135) && 
                    (b >= 120 && b <= 135) && (a >= 250)) {
                    pixelsValid = true;
                    break;
                }
            }
            
            if (pixelsValid) {
                std::cout << "  PASSED: Framebuffer color output verified" << std::endl;
                testsPassed++;
            } else {
                std::cout << "  FAILED: Framebuffer color output invalid" << std::endl;
            }
        }
        
        glDeleteTextures(1, &colorTex);
        glDeleteFramebuffers(1, &fbo);
    }
    
    // Check for OpenGL errors
    std::cout << "Test 7: Check for OpenGL errors..." << std::endl;
    {
        GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            std::cout << "  PASSED: No OpenGL errors detected" << std::endl;
            testsPassed++;
        } else {
            std::cout << "  FAILED: OpenGL error detected: 0x" << std::hex << error << std::dec << std::endl;
        }
    }
    
    // Update total tests count
    totalTests = 7;
    
    // Summary
    std::cout << std::endl;
    if (testsPassed == totalTests) {
        std::cout << "========== ALL TESTS PASSED (" << testsPassed << "/" << totalTests << ") ==========" << std::endl;
    } else {
        std::cout << "========== TESTS FAILED (" << testsPassed << "/" << totalTests << " passed) ==========" << std::endl;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return (testsPassed == totalTests) ? 0 : 1;
}
