#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 uViewProjMatrix;
uniform vec3 uEyePos;
uniform int uColumns;
uniform int uRows;
uniform float uPartWidth;
uniform float uPartHeight;

in vec2 cellPos[1];

out vec2 texCoord;

void main() {
	vec3 pos = gl_in[0].gl_Position.xyz;

	vec3 viewVec = normalize(uEyePos - pos);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(viewVec, up);



	//lower left
	pos -= (right * 0.5 * uPartWidth);
	texCoord = vec2(cellPos[0].x, 1.0 / float(uRows) + cellPos[0].y);
    gl_Position = uViewProjMatrix * vec4(pos, 1.0);
	EmitVertex();

	//upper left
	pos.y += 1.0 * uPartHeight;
	texCoord = vec2(cellPos[0].x, cellPos[0].y);
	gl_Position = uViewProjMatrix * vec4(pos, 1.0);
	EmitVertex();

	//lower right
	pos.y -= 1.0 * uPartHeight;
    pos += right * uPartWidth;
	texCoord = vec2(1.0 / float(uColumns) + cellPos[0].x, 1.0 / float(uRows) + cellPos[0].y);
	gl_Position = uViewProjMatrix * vec4(pos, 1.0);
	EmitVertex();

	//upper right
	pos.y += 1.0 * uPartHeight;
	texCoord = vec2(1.0 / float(uColumns) + cellPos[0].x, cellPos[0].y);
	gl_Position = uViewProjMatrix * vec4(pos, 1.0);
	EmitVertex();

	EndPrimitive();
}