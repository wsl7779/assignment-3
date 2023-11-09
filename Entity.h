/**
* Author: Will Lee
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

enum EntityType { S_PLATFORM, PLATFORM, V_PLATFORM, PLAYER, SCREEN, FIRE, BG };

class Entity
{
private:
    int* m_animation_on = NULL;

    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    glm::vec3 m_scale;
    float m_angle_speed;
    float m_angle;
    

    float m_width = 1.0f,
        m_height = 1.0f;

    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;
    int m_condition = 0;

public:
    EntityType m_type;
    bool m_is_active;

    bool const check_collision(Entity* other) const;
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);

    // ————— STATIC VARIABLES ————— //
    static const int SECONDS_PER_FRAME = 4;

    // ————— ANIMATION ————— //

    int m_animation_frames = 0,
        m_animation_index = 0,
        m_animation_cols = 0,
        m_animation_rows = 0;

    int* m_animation_indices = NULL;
    float m_animation_time = 0.0f;

    // ————— TRANSFORMATIONS ————— //
    float     m_speed;
    glm::vec3 m_movement;
    glm::mat4 m_model_matrix;

    GLuint    m_texture_id;

    // ————— METHODS ————— //
    Entity();
    Entity(EntityType type, bool active);
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time, Entity* collidable_entities, int entity_count);
    void follow(float delta_time, Entity* parent);
    void render(ShaderProgram* program);

    // ————— GETTERS ————— //
    glm::vec3 const get_position()     const { return m_position; };
    glm::vec3 const get_velocity()     const { return m_velocity; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    glm::vec3 const get_movement()     const { return m_movement; };
    float const get_angle_speed()   const { return m_angle_speed; };
    float const get_angle()            const { return m_angle; };
    EntityType const get_type()     const { return m_type; };
    int const get_cond()           const { return m_condition; };

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_position) { m_acceleration = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_angle_speed(float new_angle_sp) { m_angle_speed = new_angle_sp; };
    void const set_angle(float new_angle) { m_angle = new_angle; };
    void const set_scale(glm::vec3 new_scale, float new_height, float new_width) { m_scale = new_scale; m_height = new_height; m_width = new_width; };
    void const set_type(EntityType new_type, bool active) { m_type = new_type; m_is_active = active; };
    void const set_wh(float new_w, float new_h) { m_width = new_w; m_height = new_h; };

};