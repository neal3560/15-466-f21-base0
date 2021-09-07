#include "ColorTextureProgram.hpp"

#include "Mode.hpp"
#include "GL.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <cstdlib>

using namespace glm;
using namespace std;

/*
 * ShooterMode is a top-down 2d shooting game
 */

struct ShooterMode : Mode {
	ShooterMode();
	virtual ~ShooterMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- input state -----
	bool Mouse_Left = false;
	bool W_pressed = false;
	bool A_pressed = false;
	bool S_pressed = false;
	bool D_pressed = false;

	//----- game setting -----
	vec2 court_radius = vec2(8.0f, 5.0f);
	vec2 bullet_radius = vec2(0.1f, 0.1f);
	vec2 aim_radius = vec2(0.2f, 0.2f);
	float player_speed = 4.0f;
	float bullet_speed = 8.0f;
	float shoot_interval = 0.5f;
	uint max_bullets = 10;

	//----- game state -----
	vec2 pos_player = court_radius / 2.0f;
	vec2 pos_target;
	vec2 pos_aim = (vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX) * 2.0f - 1.0f) * court_radius;

	float shoot_timer = 0.0f;
	
	//----- bullet instance -----
	struct Bullet {
		Bullet(
			vec2 const& Position_, vec2 const& Direction_) :
			Position(Position_), Direction(Direction_), isActive(true){ }
		vec2 Position;
		vec2 Direction;
		bool isActive;
	};

	deque<Bullet*> bullets;

	//----- score and time -----
	uint score = 0;
	float timer = 30.0f;

	//----- opengl assets / helpers ------

	//draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "PongMode::Vertex should be packed");

	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)

};
