#pragma once

// shader source code
const char* sceneVertexShader = R"(
        #version 430 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoords = aTexCoords;
            
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

const char* sceneFragmentShader = R"(
        #version 430 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;
        
        out vec4 FragColor;
        
        uniform sampler2D screenTexture;
        uniform vec3 viewPos;
        uniform bool u_b_useLighting;
        uniform bool u_b_dualLighting;
        
        void main() {
            // basic Texture color
            vec4 texColor = texture(screenTexture, vec2(1.0 - TexCoords.x, TexCoords.y));
            
            if (u_b_useLighting) {
                // simple light
                vec3 norm = normalize(Normal);
                if(u_b_dualLighting)
                {
                    if (!gl_FrontFacing)
                         norm = -norm;
                }
                vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * vec3(0.8, 0.8, 0.8);
                
                // ambient 
                vec3 ambient = vec3(0.2, 0.2, 0.2);
                
                // specular 
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
                vec3 specular = spec * vec3(0.5);
                
                FragColor = vec4(ambient + diffuse + specular, 1.0) * texColor;
            } else {
                FragColor = texColor;
            }
        }
    )";

const char* distortionVertexShader = R"(
        #version 430 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoords;
        
        out vec2 TexCoords;
        
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoords = aTexCoords;
        }
    )";

const char* distortionFragmentShader = R"(
        #version 430 core
        in vec2 TexCoords;
        out vec4 FragColor;
        
        uniform sampler2D screenTexture;
        uniform float time;
        uniform bool u_b_applyDistortion;
        
        void main() {
            vec2 uv = TexCoords;
            
            if (u_b_applyDistortion) {
                // VR distortion effect
                vec2 center = vec2(0.5, 0.5);
                float dist = distance(uv, center);
                vec2 dir = normalize(uv - center);
                
                // barrel distortion
                float distortion = 0.15 * sin(time * 0.5) + 0.25;
                float scale = 1.0 + distortion * dist * dist;
                
                uv = center + dir * dist * scale;
                
                // Chromatic effects
                float chroma = 0.005;
                vec2 offsetR = dir * dist * chroma;
                vec2 offsetB = dir * dist * chroma * -1.0;
                
                float r = texture(screenTexture, uv + offsetR).r;
                float g = texture(screenTexture, uv).g;
                float b = texture(screenTexture, uv + offsetB).b;
                
                FragColor = vec4(r, g, b, 1.0);
            } 
            else 
            {
                FragColor = texture(screenTexture, uv);
            }
        }
    )";


