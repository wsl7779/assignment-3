/**
* Author: Will Lee
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define NUMBER_OF_ENEMIES 0
#define FIXED_TIMESTEP 0.0166666f
#define ACC_OF_GRAVITY -1.5f
#define PLATFORM_COUNT 11

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— STRUCTS AND ENUMS —————//
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* v_plat;
    Entity* s_plat;
    Entity* e_list;
    Entity* fire;
    Entity* win_sc;
    Entity* lose_sc;
    Entity* bg;
};

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH = 1080,
WINDOW_HEIGHT = 720;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "assets/alis.png";
const char PLATFORM_FILEPATH[] = "assets/rock.png";
const char END_FILEPATH[] = "assets/mars.png";
const char START_FILEPATH[] = "assets/earth.png";
const char WIN_FILEPATH[] = "assets/youwin.jpg";
const char LOSE_FILEPATH[] = "assets/youdied.jpg";
const char TEXT_FILEPATH[] = "assets/font1.png";
const char FIRE_FILEPATH[] = "assets/fire.png";
const char BG_FILEPATH[] = "assets/space.jpg";

GLuint g_text_texture_id;


const int FONTBANK_SIZE = 16;
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;
int g_condition = 0;
int fuel_amount = 1000;
bool using_fuel = false;

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}
void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Aris to Mars but the Asteroids have CRAZY Gravity in SPACE WOAH",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_game_state.e_list = new Entity[PLATFORM_COUNT + 2];

    g_text_texture_id = load_texture(TEXT_FILEPATH);

    g_game_state.bg = new Entity;
    g_game_state.bg->set_scale(glm::vec3(10.0f, 10.0f, 1.0f), 1.0f, 1.0f);
    g_game_state.bg->update(0.0f, NULL, 0);
    g_game_state.bg->m_texture_id = load_texture(BG_FILEPATH);

    // ————— PLAYER ————— //

    g_game_state.win_sc = new Entity(SCREEN, false);
    g_game_state.win_sc->set_scale(glm::vec3(3.0f, 3.0f, 1.0f), 3.0f, 3.0f);
    g_game_state.win_sc->update(0.0f, NULL, 0);
    g_game_state.win_sc->m_texture_id = load_texture(WIN_FILEPATH);

    g_game_state.lose_sc = new Entity(SCREEN, false);
    g_game_state.lose_sc->set_scale(glm::vec3(3.0f, 3.0f, 1.0f), 3.0f, 3.0f);
    g_game_state.lose_sc->update(0.0f, NULL, 0);
    g_game_state.lose_sc->m_texture_id = load_texture(LOSE_FILEPATH);

    g_game_state.player = new Entity(PLAYER, true);
    g_game_state.player->set_position(glm::vec3(-4.0f, 2.0f, 0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    g_game_state.player->set_wh(0.7f, 0.5f);
    g_game_state.player->m_speed = 1.0f;
    g_game_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    g_game_state.fire = new Entity(FIRE, false);
    g_game_state.fire->m_animation_frames = 2;
    g_game_state.fire->m_animation_cols = 2;
    g_game_state.fire->m_animation_rows = 1;
    g_game_state.fire->m_animation_indices = new int[2] {0, 1};
    g_game_state.fire->set_scale(glm::vec3(0.5f, 0.5f, 0.5f), 0.5f, 0.5f);
    g_game_state.fire->m_texture_id = load_texture(FIRE_FILEPATH);

    g_game_state.v_plat = new Entity(V_PLATFORM, true);
    g_game_state.v_plat->set_position(glm::vec3(3.0f, 1.5f, 0.0f));
    g_game_state.v_plat->set_wh(0.8f, 0.8f);
    g_game_state.v_plat->update(0.0f, NULL, 0);
    g_game_state.v_plat->m_texture_id = load_texture(END_FILEPATH);
    g_game_state.e_list[0] = g_game_state.v_plat[0];

    g_game_state.s_plat = new Entity(S_PLATFORM, true);
    g_game_state.s_plat->set_position(glm::vec3(-4.0f, -2.0f, 0.0f));
    g_game_state.v_plat->set_wh(0.8f, 0.8f);
    g_game_state.s_plat->update(0.0f, NULL, 0);
    g_game_state.s_plat->m_texture_id = load_texture(START_FILEPATH);
    g_game_state.e_list[1] = g_game_state.s_plat[0];

    g_game_state.platforms = new Entity[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_game_state.platforms[i].set_type(PLATFORM, true);
        g_game_state.platforms[i].m_texture_id = load_texture(PLATFORM_FILEPATH);
        g_game_state.platforms[i].set_position(glm::vec3(i - 5.0f, -3.5f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, 0);
        g_game_state.e_list[2 + i] = g_game_state.platforms[i];
    }

    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    g_game_state.player->set_angle_speed(glm::radians(0.0f));
    using_fuel = false;
    g_game_state.fire->m_is_active = false;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: g_game_is_running = false;
            default:     break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_game_state.player->set_angle_speed(glm::radians(60.0f));
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_game_state.player->set_angle_speed(glm::radians(-60.0f));
    }
    if (key_state[SDL_SCANCODE_UP])
    {
        if (fuel_amount > 0) {
            g_game_state.fire->m_is_active = true;
            g_game_state.player->set_acceleration(glm::vec3(-1.0f * glm::sin(g_game_state.player->get_angle()), 1.0f * glm::cos(g_game_state.player->get_angle()), 0.0f));
            using_fuel = true;
        }
    }

    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    {
        g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
    }
}

void update()
{

    if (g_condition == 2) {
        g_game_state.win_sc->m_is_active = true;
    }
    else if (g_condition == 1) {
        g_game_state.lose_sc->m_is_active = true;
    }
    else {

        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;

        // ————— FIXED TIMESTEP ————— //
        // STEP 1: Keep track of how much time has passed since last step
        delta_time += g_time_accumulator;

        // STEP 2: Accumulate the ammount of time passed while we're under our fixed timestep
        if (delta_time < FIXED_TIMESTEP)
        {
            g_time_accumulator = delta_time;
            return;
        }

        // STEP 3: Once we exceed our fixed timestep, apply that elapsed time into the objects' update function invocation
        while (delta_time >= FIXED_TIMESTEP)
        {
            // Notice that we're using FIXED_TIMESTEP as our delta time
            g_game_state.player->update(FIXED_TIMESTEP, g_game_state.e_list, PLATFORM_COUNT + 2);
            g_game_state.fire->follow(FIXED_TIMESTEP, g_game_state.player);
            delta_time -= FIXED_TIMESTEP;
        }

        g_time_accumulator = delta_time;

        g_condition = g_game_state.player->get_cond();

        if (using_fuel > 0) {
            fuel_amount -= 1 * FIXED_TIMESTEP;
        }
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.bg->render(&g_shader_program);
    g_game_state.player->render(&g_shader_program);
    g_game_state.fire->render(&g_shader_program);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        g_game_state.platforms[i].render(&g_shader_program);
    }

    g_game_state.v_plat->render(&g_shader_program);
    g_game_state.s_plat->render(&g_shader_program);
    g_game_state.win_sc->render(&g_shader_program);
    g_game_state.lose_sc->render(&g_shader_program);

    draw_text(&g_shader_program, g_text_texture_id, std::string("REMAINING FUEL:"), 0.25f, 0.0f, glm::vec3(-4.5f, 3.0f, 0.0f));
    draw_text(&g_shader_program, g_text_texture_id, std::string(std::to_string(fuel_amount)), 0.25f, 0.01f, glm::vec3(-4.0f, 2.5f, 0.0f));

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

// ————— DRIVER GAME LOOP ————— /
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}