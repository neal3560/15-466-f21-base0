#include "ShooterMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <math.h>

ShooterMode::ShooterMode() {
	
	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//set up the vertex array object to describe arrays of PongMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 0 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 + 4*1 //offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1,1);
		std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}
}

ShooterMode::~ShooterMode() {

	//----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;
}

bool ShooterMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	switch (evt.type) {

		//keyboard up
		case SDL_KEYUP:
			//cout << "KeyUp" << endl;
			switch (evt.key.keysym.sym) {
				case SDLK_a:
					A_pressed = false;
					break;
				case SDLK_d:
					D_pressed = false;
					break;
				case SDLK_w:
					W_pressed = false;
					break;
				case SDLK_s:
					S_pressed = false;
					break;
				default:
					break;
			}
			break;

		//keyboard down
		case SDL_KEYDOWN:
			//cout << "KeyDown" << endl;
			switch (evt.key.keysym.sym) {
				case SDLK_a:
					A_pressed = true;
					break;
				case SDLK_d:
					D_pressed = true;
					break;
				case SDLK_w:
					W_pressed = true;
					break;
				case SDLK_s:
					S_pressed = true;
					break;
				default:
					break;
			}
			break;

		//mouse motion
		case SDL_MOUSEMOTION:
			vec2 clip_mouse = vec2(
				(evt.motion.x + 0.5f) / window_size.x * 2.0f - 1.0f,
				(evt.motion.y + 0.5f) / window_size.y * -2.0f + 1.0f
			);
			pos_target = clip_to_court * vec3(clip_mouse, 1.0f);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (evt.button.button == SDL_BUTTON_LEFT) {
				Mouse_Left = true;
				//cout << "MouseDown" << endl;
			}			
			break;

		case SDL_MOUSEBUTTONUP:
			if (evt.button.button == SDL_BUTTON_LEFT) {
				Mouse_Left = false;
				//cout << "MouseUp" << endl;
			}
			break;

		default:
			break;
	}

	return false;
}

void ShooterMode::update(float elapsed) {

	//----- player movement -----

	if (W_pressed) {
		pos_player.y += player_speed * elapsed;
		pos_player.y = std::min(pos_player.y, court_radius.y);
	}

	if (A_pressed) {
		pos_player.x -= player_speed * elapsed;
		pos_player.x = std::max(pos_player.x, -court_radius.x);
	}

	if (S_pressed) {
		pos_player.y -= player_speed * elapsed;
		pos_player.y = std::max(pos_player.y, -court_radius.y);
	}

	if (D_pressed) {
		pos_player.x += player_speed * elapsed;
		pos_player.x = std::min(pos_player.x, court_radius.x);
	}

	//----- shoot handling -----

	if (shoot_timer > 0.0f)
		shoot_timer -= elapsed;

	if (Mouse_Left && shoot_timer <= 0.0f) {
		bullets.push_back(new Bullet(pos_player, normalize(pos_target - pos_player)));
		if (bullets.size() > max_bullets) {
			bullets.pop_front();
		}
		shoot_timer = shoot_interval;
	}

	//----- update bullets -----

	for (int i = 0; i < bullets.size(); i++) {
		if (bullets[i]->isActive) {

			//position update
			bullets[i]->Position += bullets[i]->Direction * bullet_speed * elapsed;

			//wall collision
			if (
				bullets[i]->Position.y > court_radius.y - bullet_radius.y ||
				bullets[i]->Position.y < -court_radius.y + bullet_radius.y ||
				bullets[i]->Position.x > court_radius.x - bullet_radius.x ||
				bullets[i]->Position.x < -court_radius.x + bullet_radius.x
			) {
				bullets[i]->isActive = false;
			}

			//aim collision
			if (
				bullets[i]->Position.y + bullet_radius.y > pos_aim.y - aim_radius.y &&
				bullets[i]->Position.y - bullet_radius.y < pos_aim.y + aim_radius.y &&
				bullets[i]->Position.x + bullet_radius.x > pos_aim.x - aim_radius.x &&
				bullets[i]->Position.x - bullet_radius.x < pos_aim.x + aim_radius.x 
			) {
				pos_aim = (vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX) * 2.0f - 1.0f) * court_radius;
				score++;
				bullets[i]->isActive = false;
			}
			
		}
	}

	// ----- update timer -----
	timer -= elapsed;
	if (timer <= 0.0f) {
		timer = 30.0;
		score = 0;
	}
}

void ShooterMode::draw(glm::uvec2 const &drawable_size) {
	//some nice colors from the course web page:
	#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	const glm::u8vec4 bg_color = HEX_TO_U8VEC4(0x193b59ff);
	const glm::u8vec4 fg_color = HEX_TO_U8VEC4(0xf2d2b6ff);
	const glm::u8vec4 shadow_color = HEX_TO_U8VEC4(0xf2ad94ff);
	const std::vector< glm::u8vec4 > trail_colors = {
		HEX_TO_U8VEC4(0xf2ad9488),
		HEX_TO_U8VEC4(0xf2897288),
		HEX_TO_U8VEC4(0xbacac088),
	};
	#undef HEX_TO_U8VEC4

	//other useful drawing constants:
	const float wall_radius = 0.05f;
	const float shadow_offset = 0.07f;
	const float padding = 0.14f; //padding between outside of walls and edge of window

	//---- compute vertices to draw ----

	//vertices will be accumulated into this list and then uploaded+drawn at the end of this function:
	std::vector< Vertex > vertices;

	//inline helper function for triangle drawing:
	auto draw_triangle = [&vertices](vec2 const& a, vec2 const& b, vec2 const& c, u8vec4 const& color) {
		vertices.emplace_back(vec3(a.x, a.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(vec3(b.x, b.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(vec3(c.x, c.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
	};

	//inline helper function for rectangle drawing:
	auto draw_rectangle = [&vertices](glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color) {
		//draw rectangle as two CCW-oriented triangles:
		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));

		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
	};

	//inline helper function for player drawing:
	auto draw_player = [&](vec2 const& player, vec2 const& target) {
		float angle = atan2(target.x - player.x, target.y - player.y);
		
		mat4 rotate_matrix = glm::rotate(mat4(1.0f), angle, vec3(0.0f, 0.0f, -1.0f));
		vec2 a = rotate_matrix * vec4(0.0f, 0.58, 0.0f, 0.0f);
		vec2 b = rotate_matrix * vec4(-0.5f, -0.28f, 0.0f, 0.0f);
		vec2 c = rotate_matrix * vec4(0.5f, -0.28f, 0.0f, 0.0f);
		
		draw_triangle(player + a, player + b, player, fg_color);
		draw_triangle(player + a, player + c, player, fg_color);
	};

	//inline helper function for target drawing:
	auto draw_target = [&](vec2 const& target) {
		//draw_rectangle(target, vec2(0.04f, 0.04f), fg_color);
		draw_rectangle(target + vec2(0.2f, 0.0f), vec2(0.1f, 0.03f), fg_color);
		draw_rectangle(target + vec2(-0.2f, 0.0f), vec2(0.1f, 0.03f), fg_color);
		draw_rectangle(target + vec2(0.0f, 0.2f), vec2(0.03f, 0.1f), fg_color);
		draw_rectangle(target + vec2(0.0f, -0.2f), vec2(0.03f, 0.1f), fg_color);
	};

	//player and target
	draw_player(pos_player, pos_target);
	draw_target(pos_target);

	//bullets
	for (int i = 0; i < bullets.size(); i++) {
		if (bullets[i]->isActive) {
			draw_rectangle(bullets[i]->Position, bullet_radius, fg_color);
		}
	}

	//walls:
	draw_rectangle(glm::vec2(-court_radius.x-wall_radius, 0.0f), glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2( court_radius.x+wall_radius, 0.0f), glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2( 0.0f,-court_radius.y-wall_radius), glm::vec2(court_radius.x, wall_radius), fg_color);
	draw_rectangle(glm::vec2( 0.0f, court_radius.y+wall_radius), glm::vec2(court_radius.x, wall_radius), fg_color);

	//aim
	draw_rectangle(pos_aim, aim_radius, fg_color);

	//scores and time:
	glm::vec2 score_radius = glm::vec2(0.1f, 0.1f);
	for (uint32_t i = 0; i < score; ++i) {
		draw_rectangle(glm::vec2( -court_radius.x + (2.0f + 3.0f * i) * score_radius.x, court_radius.y + 2.0f * wall_radius + 2.0f * score_radius.y), score_radius, fg_color);
	}

	//------ compute court-to-window transform ------

	//compute area that should be visible:
	glm::vec2 scene_min = glm::vec2(
		-court_radius.x - 2.0f * wall_radius - padding,
		-court_radius.y - 2.0f * wall_radius - padding
	);
	glm::vec2 scene_max = glm::vec2(
		court_radius.x + 2.0f * wall_radius + padding,
		court_radius.y + 2.0f * wall_radius + 3.0f * score_radius.y + padding
	);

	//compute window aspect ratio:
	float aspect = drawable_size.x / float(drawable_size.y);
	//we'll scale the x coordinate by 1.0 / aspect to make sure things stay square.

	//compute scale factor for court given that...
	float scale = std::min(
		(2.0f * aspect) / (scene_max.x - scene_min.x), //... x must fit in [-aspect,aspect] ...
		(2.0f) / (scene_max.y - scene_min.y) //... y must fit in [-1,1].
	);

	glm::vec2 center = 0.5f * (scene_max + scene_min);

	//build matrix that scales and translates appropriately:
	glm::mat4 court_to_clip = glm::mat4(
		glm::vec4(scale / aspect, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-center.x * (scale / aspect), -center.y * scale, 0.0f, 1.0f)
	);
	//NOTE: glm matrices are specified in *Column-Major* order,
	// so each line above is specifying a *column* of the matrix(!)

	//also build the matrix that takes clip coordinates to court coordinates (used for mouse handling):
	clip_to_court = glm::mat3x2(
		glm::vec2(aspect / scale, 0.0f),
		glm::vec2(0.0f, 1.0f / scale),
		glm::vec2(center.x, center.y)
	);

	//---- actual drawing ----

	//clear the color buffer:
	glClearColor(bg_color.r / 255.0f, bg_color.g / 255.0f, bg_color.b / 255.0f, bg_color.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	//upload vertices to vertex_buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//set color_texture_program as current program:
	glUseProgram(color_texture_program.program);

	//upload OBJECT_TO_CLIP to the proper uniform location:
	glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));

	//use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
	glBindVertexArray(vertex_buffer_for_color_texture_program);

	//bind the solid white texture to location zero so things will be drawn just with their colors:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	//run the OpenGL pipeline:
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

	//unbind the solid white texture:
	glBindTexture(GL_TEXTURE_2D, 0);

	//reset vertex array to none:
	glBindVertexArray(0);

	//reset current program to none:
	glUseProgram(0);
	

	GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.

}
