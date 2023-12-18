#version 400 compatibility

uniform sampler3D tex;

in float cylR1;
in float cylR2;
in vec3 cylDir;
in vec3 cylP0;
in vec3 cylP1;
in vec3 cylN0;
in vec3 cylN1;
in vec3 dir;

in vec3 ViewDirection;
in vec3 fvObjectPosition;
in vec3 MVPos;
in vec3 Normal;
in vec3 gc;

vec3 norm = vec3(0.0,0.0,1.0);
vec3 normBumped = vec3(0.0,0.0,1.0);
vec3 position = vec3(0.0,0.0,1.0);
vec4 color = vec4(0.0,0.0,0.0,1.0);
vec3 tc = vec3(0.0,0.0,1.0);
mat4 miMV;
vec3 rayStart = vec3(0.0,0.0,1.0);
vec3 rayDir = vec3(0.0,0.0,1.0);

#define eps 0.0001
#define mP gl_ProjectionMatrix

vec2 solveEq(float A, float B, float C) {
   	float D = B*B-4.0*A*C;
   	if (D < 0.0) discard; // no solution/intersection
   	D = sqrt(D);
   	float t1 = (-B+D)/A*0.5;
   	float t2 = (-B-D)/A*0.5;
   	return vec2(t1, t2);
}

float intersectCap(vec3 rayStart, vec3 rayDir, vec3 p0, vec3 n, float r) {
	float t = dot((p0-rayStart),n)/dot(rayDir,n);
   	vec3 p = rayStart + t*rayDir;
   	if ( abs(dot(n, p-p0)) > eps ) return -1.0;
   	if ( distance(p,p0) > r ) return -1.0;
   	return t;
}

// ellipsoid: (x/a)2 + (y/b)2 + (z/c)2 = 1
vec3 applyCaps(vec3 rayStart, vec3 rayDir, float tp) {
   	vec3 pC = rayStart + tp*rayDir;
   	if ( dot(cylN0, pC-cylP0) < -eps ) {
   		float tc0 = intersectCap(rayStart, rayDir, cylP0, cylN0, cylR1);
   		if (tc0 > eps) {
   			tp = max( tp, tc0 );
   			pC = rayStart + tp*rayDir;
   		} else discard;
   	}
   	
   	if ( dot(cylN1, pC-cylP1) >  eps )  {
   		float tc1 = intersectCap(rayStart, rayDir, cylP1, cylN1, cylR2);
   		if (tc1 > eps) {
   			tp = max( tp, tc1 );
   			pC = rayStart + tp*rayDir;
   		} else discard;
   	}
   	return pC;
}

vec3 raycastCylinder() {
   	vec3 rayDRad = rayDir - dot(rayDir, cylDir)*cylDir;
   	vec3 rayPRad = rayStart-cylP0 - dot(rayStart-cylP0, cylDir)*cylDir;
   	
   	float A = dot(rayDRad,rayDRad);
   	float B = 2.0*dot(rayDRad, rayPRad);
   	float C = dot(rayPRad, rayPRad) - cylR1*cylR1;
   	vec2 t = solveEq(A, B, C);
   	float tp = min(t[0],t[1]);
   	return applyCaps(rayStart, rayDir, tp);
}

vec3 raycastCone() {
   	float H = distance(cylP0, cylP1);
   	float H2 = H*H;
   	float dR = cylR1-cylR2;
   	float dR2 = dR*dR;
   	
   	if (abs(dR) < eps) return raycastCylinder();
   	//return fvObjectPosition;
   	
   	vec3 cylPa = cylP0 + cylDir * cylR1*H/dR;
   	float vrvc = dot(rayDir, cylDir);
   	float dpvc = dot(rayStart-cylPa, cylDir);
   	vec3 rayDRad = rayDir - vrvc*cylDir;
   	vec3 rayPRad = rayStart-cylPa - dpvc*cylDir;
   	float cos2a = H2 / (H2 + dR2);
   	float sin2a = 1.0 - cos2a;
   	
   	float A = cos2a * dot(rayDRad,rayDRad) - sin2a * vrvc * vrvc;
   	float B = 2.0 * (cos2a * dot(rayDRad, rayPRad) - sin2a * vrvc * dpvc);
   	float C = cos2a * dot(rayPRad, rayPRad) - sin2a * dpvc * dpvc;
   	vec2 t = solveEq(A, B, C);
   	float tp = min(t[0],t[1]);
   	return applyCaps(rayStart, rayDir, tp);
}

const vec2 size = vec2(0.1,-0.1);
const ivec3 off = ivec3(-1,0,1);
void applyBumpMap() {
	float s11 = texture(tex, tc).r;
	float s01 = textureOffset(tex, tc, off.xyz).r;
	float s21 = textureOffset(tex, tc, off.zyx).r;
	float s10 = textureOffset(tex, tc, off.yxz).r;
	float s12 = textureOffset(tex, tc, off.yzx).r;

	vec3 va = normalize(vec3(size.xy,s21-s01));
	vec3 vb = normalize(vec3(size.yx,s12-s10));
	normBumped = normalize(norm+0.8*cross(va,vb));
	//norm = cross(va,vb);
}

void computeNormal() {
   	norm = position - cylP0 - dot(position - cylP0, cylDir)*cylDir;
	norm = (miMV*vec4(norm, 0.0)).xyz;
   	norm = normalize(norm);
}

void computeTexCoords() {
	float h = dot(position-cylP0, cylDir);
	tc = vec3(norm.x+h*0.5, h*0.3/cylR1, norm.z+h*0.5);
}

void computeDepth() {
	vec4 pp = mP * vec4(position, 1);
	float d = pp.z / pp.w;
	gl_FragDepth = d*0.5 + 0.5;
}

void applyBlinnPhong() {
	vec3  light = normalize( gl_LightSource[0].position.xyz );// directional light
	color.xyz *= abs(dot( norm, normBumped )); // apply bumps to color
	norm = normalize( gl_NormalMatrix * normBumped );
	float NdotL = max(dot( norm, light ), 0.0);
	vec4  ambient = gl_LightSource[0].ambient * color;
	vec4  diffuse = gl_LightSource[0].diffuse * NdotL * color;
	float NdotHV = max(dot(norm, normalize(gl_LightSource[0].halfVector.xyz)),0.0);
	vec4  specular = mix(gl_FrontMaterial.specular, gl_LightSource[0].specular, 0.5) * pow( NdotHV, gl_FrontMaterial.shininess ); 
	gl_FragColor = ambient + diffuse + specular;
}

void main( void ) {
	miMV = inverse( gl_ModelViewMatrix ); // TODO: avoid computing inverse in shader!
	norm = Normal;
   	rayStart = vec3(0.0);
   	rayDir = MVPos;
   	rayDir = normalize(rayDir);
   	
	position = fvObjectPosition;
	position = raycastCone();
	computeNormal();
	computeTexCoords();
	applyBumpMap();
	//color = vec4(1,0,0,1);
	//color = texture(tex, tc);
	color = vec4(gc, 1.0);
	applyBlinnPhong();
	//gl_FragColor = vec4(gc, 1.0);
	computeDepth();
}




