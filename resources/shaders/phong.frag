#version 330 core

struct SceneLightData {
    int type;          // 0 = Directional, 1 = Point, 2 = Spot
    vec3 color;        // Light color (RG). Can exceed 1.0 for HDR
    vec3 function;     // Attenuation: constant, linear, quadratic
    vec4 pos;          // World-space position (point/spot)
    vec4 dir;          // World-space direction (directional/spot)
    float penumbra;    // Spot penumbra (radians)
    float angle;       // Spot angle (radians)
};

struct SceneShapeData {
    int primitive;     // The type of shape
    mat4 invCTM;       // inverse of the object's CTM
    vec3 ambient;      // object material ambient term
    vec3 diffuse;      // object material diffuse term
    vec3 specular;     // object material specular term
    float shininess;   // material shininess
    float blend;
    bool hasTexture;
    bool useTime;
};

struct rayState {
    vec3 ray_pos;
    vec3 ray_vel;
};

const int MAX_LIGHTS = 8;
uniform SceneLightData lights[MAX_LIGHTS];
uniform int numLights;

const int MAX_SHAPES = 16;
uniform SceneShapeData shapes[MAX_SHAPES];
uniform int numShapes;

uniform sampler2D uShapeTex[8];    // bound to texture units 0..7
uniform int uShapeTexUnits[8];     // mapping from shape index → unit

//Shape type definitions
const int CUBE = 0;
const int CONE = 1;
const int CYLINDER = 2;
const int SPHERE = 3;
const int MANDEL_BULB = 4;
const int MENGER = 5;
const int JULIA = 6;
const int TERRAIN = 7;
const int SPHERE_TORUS = 8;

uniform int hasBH;
uniform vec3 bh_pos;
uniform float bh_r;

uniform uint uFrameIndex;

//Light type definitions
const int LIGHT_DIRECTIONAL = 0;
const int LIGHT_POINT = 1;
const int LIGHT_SPOT = 2;

in vec2 UV;
out vec4 fragColor;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform vec3 uCameraPos;
uniform sampler2D uBackgroundTex;

// Global light coefficients
uniform float ka; // ambient
uniform float kd; // diffuse
uniform float ks; // specular

//////////// Fractal SDF definitions ////////////////
// Cube: side length 1, centered at origin
float cubeSDF(vec3 p)
{
    vec3 d = abs(p) - vec3(0.5);
    return length(max(d, 0.0)) + min(max(d.x, max(d.y,d.z)), 0.0);
}

// Sphere: radius 0.5
float sphereSDF(vec3 p)
{
    return length(p) - 0.5;
}

// Cylinder: height 1, radius 0.5
float cylinderSDF(vec3 p)
{
    vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(0.5, 0.5);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

// Cone: height 1, bottom radius 0.5
// Apex at y=+0.5, base at y=-0.5
float coneSDF(vec3 p)
{
    float shifted_y = p.y - 0.5f;

    float r_shifted = pow(pow(p.x, 2.f) + pow(p.z, 2.f), 0.5f);
    vec2 base = vec2(0.5f, -1.f);
    vec2 pos_rh = vec2(r_shifted, shifted_y);

    vec2 a = pos_rh - base * clamp(dot(pos_rh, base) / dot(base, base), 0.f, 1.f);
    float clamped_r = clamp(pos_rh.x / base.x, 0.f, 1.f);
    vec2 b = pos_rh - base * vec2(clamped_r, 1.f);

    float d = min(dot(a, a), dot(b, b));
    float s = max(pos_rh.y * base.x - pos_rh.x * base.y, base.y - pos_rh.y);

    return pow(d, 0.5f) * sign(s);
}

// Mandel buld fractral
float mandelbulbDE(vec3 pos)
{
    vec3 z = pos;
    float dr = 1.0;
    float r  = 0.0;

    const int Iterations = 12;
    const float Power   = 8.0;

    for (int i = 0; i < Iterations; i++) {
        r = length(z);
        if (r > 2.0) break;

        float theta = acos(z.z / r);
        float phi   = atan(z.y, z.x);

        float rPow = pow(r, Power - 1.0);
        dr = rPow * dr * Power + 1.0;

        float zr = pow(r, Power);
        theta *= Power;
        phi   *= Power;

        z = zr * vec3(
            sin(theta) * cos(phi),
            sin(theta) * sin(phi),
            cos(theta)
        );

        z += pos;
    }

    return 0.5 * log(r) * r / dr;
}


//Menger Sponge
float mengerSDF(vec3 p)
{
    float scale = 3.0;
    float d = -1.0;

    for (int i = 0; i < 4; i++)
    {
        p = abs(p);                 // fold into first octant
        if (p.x < p.y) p.xy = p.yx;
        if (p.x < p.z) p.xz = p.zx;

        float c = max(p.y, p.z);
        d = max(d, (c - (scale - 1.0)) / pow(scale, float(i)));

        p = p * scale - vec3(scale-1.0); // scale and translate
    }

    // distance to unit cube
    float cube_d = (length(max(p - vec3(1.0), 0.0))) / pow(scale, 4.0);
    return max(cube_d, d);
}


// Julian quaterion
float juliaDE(vec3 p)
{
    vec4 z = vec4(p, 0.0);
    const vec4 c = vec4(-0.2, 0.7, 0.3, -0.1);

    float dr = 1.0;
    float r;

    for (int i = 0; i < 10; i++) {
        r = length(z);
        if (r > 2.0) break;

        // quaternion square
        vec4 z2 = z;
        z = vec4(
            z2.x*z2.x - dot(z2.yzw, z2.yzw),
            2.0*z2.x * z2.yzw
        );
        z += c;

        dr = dr * 2.0 * r + 1.0;
    }

    return 0.5 * log(r) * r / dr;
}

//Terrain
float terrainSDF(vec3 p)
{
    float h = sin(p.x * 0.3) * cos(p.z * 0.3) * 2.0; // height oscillates 0 → ±2
    return p.y - h; // distance from point to terrain surface
}

// Smooth sphere + torus
float sphereTorusSDF(vec3 p)
{
    float sphere = length(p) - 1.0;
    float t = length(vec2(length(p.xz) - 2.0, p.y)) - 0.4;
    return min(sphere, t);
}

float getMaxScaleFactor(mat4 ctm_inv) {
    float scaleX = length(vec3(ctm_inv[0]));
    float scaleY = length(vec3(ctm_inv[1]));
    float scaleZ = length(vec3(ctm_inv[2]));

    return max(scaleX, max(scaleY, scaleZ));
}

vec2 sceneSDF(vec3 p)
{
    float minDist = 1e9;
    int shapeIndex = -1;

    for (int i = 0; i < numShapes; i++) {
        vec3 local = (shapes[i].invCTM * vec4(p, 1.0)).xyz;

        float d;
        switch (shapes[i].primitive) {
            case SPHERE:
                d = sphereSDF(local);
                break;
            case CUBE:
                d = cubeSDF(local);
                break;
            case CYLINDER:
                d = cylinderSDF(local);
                break;
            case CONE:
                d = coneSDF(local);
                break;
            case MANDEL_BULB:
                d = mandelbulbDE(local);
                break;
            case MENGER:
                d = mengerSDF(local);
                break;
            case JULIA:
                d = juliaDE(local);
                break;
            case TERRAIN:
                d = terrainSDF(local);
                break;
            case SPHERE_TORUS:
                d = sphereTorusSDF(local);
                break;
        }

        d = d / getMaxScaleFactor(shapes[i].invCTM);

        if (d < minDist) {
            minDist = d;
            shapeIndex = i;
        }
    }

    return vec2(minDist, shapeIndex);
}


//Estimates the normals for lighting calculation
vec3 estimateNormal(vec3 p)
{
    float e = 0.0001;

    return normalize(vec3(
        sceneSDF(p + vec3(e,0,0)).x -
        sceneSDF(p - vec3(e,0,0)).x,
        sceneSDF(p + vec3(0,e,0)).x -
        sceneSDF(p - vec3(0,e,0)).x,
        sceneSDF(p + vec3(0,0,e)).x -
        sceneSDF(p - vec3(0,0,e)).x
    ));
}

/*
NOTE: we need to implement the function such that we know what shape it hits,
    and so that SceneSDF gives the distance to the closest shape. this way, we can compute
    lighting. Also, we need to pass the black hole parameters as uniforms to the shader.
*/

vec3 get_accel(vec3 r, vec3 v, float bh_r) {
    vec3 h = cross(r, v);

    float r_len = length(r);
    float h_factor = -1.5 * bh_r * dot(h, h);
    vec3 a = r * (h_factor / pow(r_len, 5));

    return a;
}

rayState RK4Step(rayState ray_state, float dt, vec3 bh_pos, float bh_r) {
    vec3 r_t = ray_state.ray_pos - bh_pos;
    vec3 v_t = ray_state.ray_vel;

    // k factors and resulting interpolations. Note k_1r = v_t, k_2r = v_1_2...
    vec3 k_1v = get_accel(r_t, v_t, bh_r);
    vec3 r_1_2 = r_t + v_t * dt / 2.f;
    vec3 v_1_2 = v_t + k_1v * dt / 2.f;

    vec3 k_2v = get_accel(r_1_2, v_1_2, bh_r);
    vec3 r_2_3 = r_1_2 + v_1_2 * dt / 2.f;
    vec3 v_2_3 = v_1_2 + k_2v * dt / 2.f;

    vec3 k_3v = get_accel(r_2_3, v_2_3, bh_r);
    vec3 r_3_4 = r_2_3 + v_2_3 * dt;
    vec3 v_3_4 = v_2_3 + k_3v * dt;

    vec3 k_4v = get_accel(r_3_4, v_3_4, bh_r);

    r_t += dt / 6.f * (v_t + 2.f * v_1_2 + 2.f * v_2_3 + v_3_4);
    v_t += dt / 6.f * (k_1v + 2.f * k_2v + 2.f * k_3v + k_4v);

    rayState next_ray_state;
    next_ray_state.ray_pos = r_t + bh_pos;
    next_ray_state.ray_vel = v_t;

    return next_ray_state;
}

float marchRay(vec3 ro, vec3 rd, out vec3 hitPos, out int hitShape, out bool inside)
{
    const float MAX_DIST = 100.0;
    const float EPS = 0.0005;
    const int MAX_STEPS = 1000;
    const float LIPSCHITZ_C = 1.5;

    rayState ray_state;
    ray_state.ray_pos = ro;
    ray_state.ray_vel = rd;

    float t = 0.0;
    hitShape = -1;

    if (hasBH == 0) {
        for (int i = 0; i < MAX_STEPS; i++) {
            vec3 p = ro + rd * t;

            vec2 res = sceneSDF(p);
            float d = res.x;

            if (abs(d) < EPS) {
                hitPos = p;
                hitShape = int(res.y);

                inside = d < 0;

                // Linear approx in short distances
                return t;
            }

            if (t > MAX_DIST) break;

            float step = max(d * 0.7, 0.0005);
            t += step;
        }
    } else {
        for (int i = 0; i < MAX_STEPS; i++) {
            vec2 res = sceneSDF(ray_state.ray_pos);
            float d = res.x;

            if (d < EPS) {
                hitPos = ray_state.ray_pos;
                hitShape = int(res.y);

                // Linear approx in short distances
                return t;
            }

            if (t > MAX_DIST) break;

            if (length(ray_state.ray_pos - bh_pos) < bh_r * 1.3) break;

            float step = max(EPS, d * 0.7 / LIPSCHITZ_C);
            vec3 prev_pos = ray_state.ray_pos;
            ray_state = RK4Step(ray_state, step, bh_pos, bh_r);

            t += length(prev_pos - ray_state.ray_pos);
        }
    }

    hitShape = -1;
    hitPos = ro + rd * t;
    return t;
}

//UV definitions
vec2 sphereUV(vec3 p) {
    // Transform to local space if needed
    vec3 local = normalize(p); // assuming p is in sphere local space
    float u = 0.5 + atan(local.z, local.x) / (2.0 * 3.14159265);
    float v = 0.5 - asin(local.y) / 3.14159265;
    return vec2(u, v);
}

vec2 cubeUV(vec3 p) {
    // p is local-space point on the cube surface, in [-0.5, +0.5]

    vec3 a = abs(p);

    vec2 uv;

    // +X face
    if (a.x >= a.y && a.x >= a.z) {
        uv = p.zy;       // use z, y
        uv.y = -uv.y;    // flip for consistency
        uv = uv * 0.5 + 0.5;
    }
    // +Y face
    else if (a.y >= a.x && a.y >= a.z) {
        uv = p.xz;       // use x, z
        uv.x = -uv.x;
        uv = uv * 0.5 + 0.5;
    }
    // +Z face
    else {
        uv = p.xy;       // use x, y
        uv = uv * 0.5 + 0.5;
    }

    return uv;
}

vec2 cylinderUV(vec3 p)
{
    vec3 a = abs(p);
    float l = length(a.xz);
    if (a.y >= l && a.y >= l) {
        vec2 uv = p.xz;
        uv.x = -uv.x;
        return uv + 0.5;
    }

    // angle around Y axis
    float angle = atan(p.z, p.x);           // [-pi, pi]
    float u = (angle / (2.0 * 3.14159265));

    // y mapped from [-0.5, 0.5] -> [0,1]
    float v = clamp(p.y + 0.5, 0.0, 1.0);

    return vec2(1.0 - u, 1.0 - v);
}

vec2 coneUV(vec3 p)
{
    // --- Angular coordinate ---
    float angle = atan(p.z, p.x);         // [-pi,pi]
    float u = (angle / (2.0 * 3.14159265)) + 0.5;

    // --- V coordinate (slope distance) ---
    // shift so apex at y = 1.0 and base at y=0.0
    float h = p.y + 0.5;                  // h in [0,1]

    // slope length = sqrt(height^2 + radius^2)
    float slopeLen = sqrt(1.0 + 0.5*0.5); // ≈ 1.118

    // radial fraction along the cone
    float r = length(p.xz);               // 0 → 0.5
    float frac = r / 0.5;                 // 0 → 1

    float v = frac * (1.0 / slopeLen);    // normalized slope distance

    return vec2(u, v);
}

// Rotate a UV coordinate around a center point
vec2 rotateUV(vec2 uv, vec2 center, float angle) {
    vec2 uvRel = uv - center;
    float c = cos(angle);
    float s = sin(angle);
    uvRel = vec2(
        uvRel.x * c - uvRel.y * s,
        uvRel.x * s + uvRel.y * c
    );
    return uvRel + center;
}

vec3 computeDiffuse(vec3 pos, vec3 N, vec3 L, int shapeIndex) {
    float diff = max(dot(N, L), 0.0);

    // Base diffuse
    vec3 baseDiffuse = shapes[shapeIndex].diffuse;

    // If no texture → standard diffuse
    if (!shapes[shapeIndex].hasTexture) {
        return kd * diff * baseDiffuse;
    }

    // --- Sphere texture lookup ---
    vec3 texColor = vec3(1.0);
    vec3 local = (shapes[shapeIndex].invCTM * vec4(pos, 1.0)).xyz;
    vec2 uv = vec2(0.0);

    switch(shapes[shapeIndex].primitive) {

        case SPHERE:
            uv = sphereUV(local);
            break;

        case CUBE:
            uv = cubeUV(local);
            break;

        case CONE:
            uv = coneUV(local);
            break;

        case CYLINDER:
            uv = cylinderUV(local);
            break;
    }

    if (shapes[shapeIndex].useTime) {
        float speed = 0.01; // radians per frame
        float angle = float(uFrameIndex) * speed;
        uv = rotateUV(uv, vec2(0.0), angle);
    }

    texColor = texture(uShapeTex[shapeIndex], uv).rgb;

    // Blend between texture and base diffuse
    float blend = shapes[shapeIndex].blend;

    vec3 blendedDiffuse = (1.0 - blend) * kd * baseDiffuse + blend * texColor;

    return diff * blendedDiffuse;
}


vec3 computeLight(SceneLightData light, vec3 pos, vec3 normal, int shapeIndex) {
    vec3 N = normalize(normal);
    vec3 V = normalize(uCameraPos - pos);
    vec3 L;
    float attenuation = 1.0;

    if (light.type == LIGHT_DIRECTIONAL) { // directional
        L = normalize(-light.dir.xyz);
    } else {
        L = normalize(light.pos.xyz - pos);
        float dist = length(light.pos.xyz - pos);
        attenuation = 1.0 / (light.function.x +
                             light.function.y * dist +
                             light.function.z * dist * dist);

        if (light.type == LIGHT_SPOT) { // spot
            float theta = dot(normalize(-L), normalize(light.dir.xyz));
            float cosInner = cos(light.angle - light.penumbra);
            float cosOuter = cos(light.angle);
            float intensity = clamp((theta - cosOuter) / (cosInner - cosOuter), 0.0, 1.0);
            attenuation *= intensity;
        }
    }

    // Diffuse
    vec3 diffuse = computeDiffuse(pos, N, L, shapeIndex);

    // Specular
    vec3 specular = vec3(0.0);
    if (shapes[shapeIndex].shininess > 0.0) {
        vec3 R = reflect(-L, N);
        float spec = pow(max(dot(R, V), 0.0), shapes[shapeIndex].shininess);
        specular = ks * spec * shapes[shapeIndex].specular;
    }

    return attenuation * light.color * (diffuse + specular);
}

vec3 phongLighting(vec3 p, vec3 N, int shapeIndex) {
    // Global ambient (shape-dependent)
    vec3 color = ka * shapes[shapeIndex].ambient;

    // Add diffuse + specular per light
    for (int i = 0; i < numLights; ++i) {
        color += computeLight(lights[i], p, N, shapeIndex);
    }

    return color;
}

//For our background texture
vec2 cylindricalUV(vec3 dir) {
    vec3 d = normalize(dir);

    // azimuth angle around Y-axis (longitude)
    float theta = atan(d.z, d.x); // [-pi, pi]

    // Map theta [-pi, pi] → [0,1]
    float u = (theta + 3.14159265) / (2.0 * 3.14159265);

    // height along Y-axis [-1,1] → [0,1]
    float v = (d.y + 1.0) * 0.5;

    return vec2(u, v);
}

void main() {
    vec2 uv = UV * 2.0 - 1.0;

    /** Shoot a ray starting from each pixel position on the screen **/
    vec4 rayClip = vec4(uv, -1.0, 1.0); // Build a point on the near plane in clip space
    vec4 rayEye  = uInvProj * rayClip; //Get the same point in camera space
    rayEye = vec4(rayEye.xy, -1.0, 0.0);  // Turn that point into a direction pointing forward (-Z)
    vec4 rd = normalize(uInvView * rayEye); // Convert direction to world space
    vec3 ro = uCameraPos;  // Ray origin is the camera position

    // Ray march
    vec3 hitPos;
    int shapeIndex;
    bool inside;
    marchRay(ro, rd.xyz, hitPos, shapeIndex, inside);

    if (shapeIndex != -1) {
        vec3 N = estimateNormal(hitPos);
        if (inside) {
            N = -N;
            hitPos += N * 0.002;
        }

        vec3 color = phongLighting(hitPos, N, shapeIndex);
        fragColor = vec4(color, 1.0);
    } else {
        fragColor = vec4(vec3(0.0), 1.0);
    }
}
