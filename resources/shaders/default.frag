#version 330 core

in vec3 w_pos;
in vec3 w_norm;

// Task 10: declare an out vec4 for your output color
out vec4 fragColor;

uniform vec3 cam;

// Task 12: declare relevant uniform(s) here, for ambient lighting
uniform float ka;

// Task 13: declare relevant uniform(s) here, for diffuse lighting
uniform float kd;

// Task 14: declare relevant uniform(s) here, for specular lighting
uniform float ks;

uniform float shininess;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;

struct Light {
   vec3 pos;
   vec3 dir;
   vec3 color;
   vec3 function;
   float penumbra;
   float angle;
   int type;
};

uniform int lightCount;
uniform Light lights[8];

void main() {
   vec3 ambC = vec3(ambient) * ka;
   vec3 difC = vec3(diffuse) * kd;
   vec3 speC = vec3(specular) * ks;
   vec3 illumination = ambC;

   vec3 n_w_norm = normalize(w_norm);

   for (int i = 0; i < lightCount; i++) {
      float ang_att = 1.0f;
      float att = 1.0f;

      vec3 position = lights[i].pos;
      vec3 surfToLight = normalize(lights[i].pos - w_pos);

      if (lights[i].type == 1) {
         surfToLight = -lights[i].dir;
         position = w_pos;
      }

      if (lights[i].type == 2) {
         float angle = acos(dot(-surfToLight, lights[i].dir));
         float inner = lights[i].angle - lights[i].penumbra;

         if (angle > inner) {
             float a = (angle - inner) / (lights[i].penumbra);
             float falloff = -2.0f * (a*a*a) + 3.0f * (a*a);

             ang_att = 1.0f - falloff;
         }

         if (angle > lights[i].angle) {
            ang_att = 0.0f;
         }
      }

      float distToLight = length(w_pos - position);
      att = min(1.0f, 1.0f / (lights[i].function.x + lights[i].function.y * distToLight + lights[i].function.z * distToLight * distToLight));
      att *= ang_att;

      vec3 surfToCam = normalize(cam - w_pos);
      vec3 reflected = reflect(-surfToLight, n_w_norm);

      illumination += att * lights[i].color * difC * clamp((dot(n_w_norm, surfToLight)), 0.0f, 1.0f);
      illumination += att * lights[i].color * speC * pow(clamp(dot(reflected, surfToCam), 0.01f, 1.0f), shininess);
   }

   fragColor = vec4(illumination, 1.0f);
}
