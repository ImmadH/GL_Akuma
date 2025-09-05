#include "camera.h"
void update_camera_vectors(Camera* camera)
{

    vec3 front;
    front[0] = cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    front[1] = sinf(glm_rad(camera->pitch));
    front[2] = sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    glm_normalize_to(front, camera->front);

    glm_vec3_cross(camera->front, camera->world_up, camera->right);
    glm_normalize(camera->right);

    glm_vec3_cross(camera->right, camera->front, camera->up);
    glm_normalize(camera->up);
}

void camera_init(Camera* camera, vec3 camPos, vec3 up, float yaw, float pitch) 
{
    glm_vec3_copy(camPos, camera->position);
    glm_vec3_copy(up, camera->world_up);
    camera->yaw = yaw;
    camera->pitch = pitch;
    camera->speed = 5.0f;
    camera->sensitivity = 0.5f;
    update_camera_vectors(camera);
}

void camera_process_keyboard(Camera* camera, cameraMovement direction, float deltaTime)
{
    float velocity = camera->speed * deltaTime;
    vec3 delta;

    switch (direction) {
        case FORWARD:
            glm_vec3_scale(camera->front, velocity, delta);
            glm_vec3_add(camera->position, delta, camera->position);
            break;
        case BACKWARD:
            glm_vec3_scale(camera->front, velocity, delta);
            glm_vec3_sub(camera->position, delta, camera->position);
            break;
        case LEFT:
            glm_vec3_scale(camera->right, velocity, delta);
            glm_vec3_sub(camera->position, delta, camera->position);
            break;
        case RIGHT:
            glm_vec3_scale(camera->right, velocity, delta);
            glm_vec3_add(camera->position, delta, camera->position);
            break;
        case UP:
            glm_vec3_scale(camera->world_up, velocity, delta);
            glm_vec3_add(camera->position, delta, camera->position);
            break;
        case DOWN:
            glm_vec3_scale(camera->world_up, velocity, delta);
            glm_vec3_sub(camera->position, delta, camera->position);
            break;
    }
}

void camera_process_mouse(Camera* camera, float xoffset, float yoffset) 
{
    xoffset *= camera->sensitivity;
    yoffset *= camera->sensitivity;

    camera->yaw += xoffset;
    camera->pitch += yoffset;

    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    update_camera_vectors(camera);
}

void camera_get_view_matrix(Camera* camera, mat4 dest) 
{
    vec3 center;
    glm_vec3_add(camera->position, camera->front, center);
    glm_lookat(camera->position, center, camera->up, dest);
}


void camera_update_projection(Camera* camera)
{
  glm_perspective(glm_rad(camera->fov_deg), camera->aspect, camera->znear, camera->zfar, camera->projection);
}

void camera_set_perspective(Camera* camera, float fov_deg, float aspect, float znear, float zfar)
{
  camera->fov_deg = fov_deg;
  camera->aspect  = aspect;
  camera->znear   = znear;
  camera->zfar    = zfar;
  camera_update_projection(camera);
}

