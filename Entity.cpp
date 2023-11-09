/**
* Author: Will Lee
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

Entity::Entity()
{
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);
    m_movement = glm::vec3(0.0f);
    m_speed = 0;
    m_model_matrix = glm::mat4(1.0f);
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    m_angle = glm::radians(0.0f);
    m_angle_speed = 0;
    m_is_active = true;
}

Entity::Entity(EntityType type, bool active)
{
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);
    m_movement = glm::vec3(0.0f);
    m_speed = 0;
    m_model_matrix = glm::mat4(1.0f);
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    m_angle = glm::radians(0.0f);
    m_angle_speed = 0;
    m_type = type;
    m_is_active = active;
}

Entity::~Entity()
{
    delete[] m_animation_on;
}

bool const Entity::check_collision(Entity* other) const
{
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


void Entity::update(float delta_time, Entity* collidable_entities, int collidable_entity_count)
{
    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    check_collision_y(collidable_entities, collidable_entity_count);
    check_collision_x(collidable_entities, collidable_entity_count);


    if (m_animation_indices != NULL)
    {
        if (glm::length(m_movement) != 0 || m_type == FIRE)
        {
            m_animation_time += delta_time;
            float frames_per_second = (float)1 / SECONDS_PER_FRAME;

            if (m_animation_time >= frames_per_second)
            {
                m_animation_time = 0.0f;
                m_animation_index++;

                if (m_animation_index >= m_animation_frames)
                {
                    m_animation_index = 0;
                }
            }
        }
    }

    m_angle += m_angle_speed * delta_time;

    // And we add the gravity next
    m_velocity += m_acceleration * delta_time;
    m_position += m_velocity * delta_time;

    // Apply transformation
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::rotate(m_model_matrix, m_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
}

void Entity::follow(float delta_time, Entity* parent)
{
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, parent->m_position);
    m_model_matrix = glm::rotate(m_model_matrix, parent->m_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    m_model_matrix = glm::translate(m_model_matrix, glm::vec3(0.0f, -0.5f, 0.0f));
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0) {
                m_position.y -= y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_top = true;
                m_condition = 1;
            }
            else if (m_velocity.y < 0) {
                m_position.y += y_overlap;
                m_velocity.y = 0;
                m_velocity.x = 0;

                // Collision!
                m_collided_bottom = true;
                if (collidable_entity->m_type == V_PLATFORM) {
                    m_condition = 2;
                }
                else if (collidable_entity->m_type == PLATFORM) {
                   m_condition = 1;
                }
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    if (!m_is_active) return;
        for (int i = 0; i < collidable_entity_count; i++)
        {
            Entity* collidable_entity = &collidable_entities[i];

            if (check_collision(collidable_entity))
            {
                float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
                float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
                if (m_velocity.x > 0) {
                    m_position.x -= x_overlap;
                    m_velocity.x = 0;

                    // Collision!
                    m_collided_right = true;
                }
                else if (m_velocity.x < 0) {
                    m_position.x += x_overlap;
                    m_velocity.x = 0;

                    // Collision!
                    m_collided_left = true;
                }
                m_condition = 1;
            }
        }
}

void Entity::render(ShaderProgram* program)
{
    if (!m_is_active) return;

    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}