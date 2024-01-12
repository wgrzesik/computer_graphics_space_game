#version 430 core

uniform vec3 color;

in vec3 interpNormal;

out vec4 out_color;

void main()
{
	
    float rimIntensity = 0.5;  // intensywnoœæ
    vec3 rimColor = vec3(1.0, 0.0, 0.0);  // kolor rim
    float rim = dot(normalize(interpNormal), normalize(vec3(0.0, 0.0, 1.0)));  // wektor (0, 0, 1) ¿eby na krawêdziach
    rim = 1 - pow(rim, 2);  // jak szeroki jest ten rim

    vec3 finalColor = color;

    finalColor = mix(color, rimColor, rimIntensity * rim);


	out_color = vec4(finalColor, 1.0);

}
