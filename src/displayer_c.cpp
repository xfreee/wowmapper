#include "displayer_c.h"
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/type_ptr.hpp>

DisplayCallbacks_t Displayer_c::display_callbacks_;
Camera_c Displayer_c::camera_;
glm::vec2 Displayer_c::mouse_pos_(0.0f, 0.0f);
const AdtList_t *Displayer_c::adt_list_= NULL;

glm::vec3 *Displayer_c::vertices_ = NULL;
glm::vec3 *Displayer_c::normales_ = NULL;

#define XZOFFSET 533.333333333f

Displayer_c::Displayer_c(int32_t width, int32_t height, const char *title)
    : win_id_(0),
      width_(width),
      height_(height) {

  glutInit(&win_id_, NULL);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);

  win_id_ = glutCreateWindow(title);
  glutDisplayFunc(Displayer_c::RenderLoop);
  glutIdleFunc(Displayer_c::RenderLoop);

  /* set keyboard and mouse callbacks */
  glutPassiveMotionFunc(Displayer_c::PassiveMouseCallback);
  glutMotionFunc(Displayer_c::MouseCallback);
  glutKeyboardFunc(Displayer_c::KeyboardCallback);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glShadeModel(GL_SMOOTH);
  //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  GLfloat LightAmbient[]= { 0.4f, 0.4f, 0.2f, 1.0f };
  GLfloat LightDiffuse[]= { 0.5f, 0.5f, 0.5f, 0.5f };
  GLfloat LightPosition[]= { 0.0f, 10000.0f, 0.0f, 0.5f };
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearColor(0, 0, 0.1, 1.0f);
  //glEnable(GL_NORMALIZE);
}

Displayer_c::~Displayer_c() {
  glutDestroyWindow(win_id_);
}

void Displayer_c::Start(AdtList_t *adtList) {
  adt_list_ = adtList;

  vertices_ = new glm::vec3[adtList->size() * 12 * 64 *256];
  normales_ = new glm::vec3[adtList->size() * 12 * 64 *256];

  int counter = 0;
  for(AdtList_t::const_iterator adt = adt_list_->begin();
          adt != adt_list_->end();
          ++adt, counter++) {
    for(int i = 0; i < 12*64*256; i++) {
      vertices_[counter*12*64*256+i] = (*adt)->vertices()[i];
      normales_[counter*12*64*256+i] = (*adt)->normals()[i];
    }
  }

  glutMainLoop();
}

void Displayer_c::RenderLoop() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  gluPerspective(60, 4/3.0f, 1.0, 100000.0);

  glm::vec3 world_trans = adt_list_->front()->mhdr_chunk().mcin->mcnk_index[0].mcnk->position;

  glm::vec3 curr_pos = camera_.position();
  glm::vec3 look_at = curr_pos + camera_.direction();
  gluLookAt(curr_pos.x, curr_pos.y, curr_pos.z,
            look_at.x, look_at.y, look_at.z,
            0.0f, 1.0f, 0.0f);

  std::vector<uint32_t> uids;

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  for (AdtList_t::const_iterator adt = adt_list_->begin();
       adt != adt_list_->end();
       ++adt) {

    const ModfChunk_s *modf = (*adt)->mhdr_chunk().modf;

    for(uint32_t i = 0; i < modf->wmo_info.size(); i++) {
      glm::vec3 pos = modf->wmo_info[i].position;
      glm::vec3 rot = modf->wmo_info[i].orientation;

      uint32_t counter = 0;
      /* draw wmo if id matches */
      for (WmoList_t::const_iterator wmo = (*adt)->wmo_list().begin();
           wmo != (*adt)->wmo_list().end();
           ++wmo, counter++) {

        if (modf->wmo_info[i].wmo_id == counter &&
            (std::find(uids.begin(), uids.end(), modf->wmo_info[i].uid) == uids.end())) {

          uids.push_back(modf->wmo_info[i].uid);
          //std::cout << rot.x << " " << rot.y << " " << rot.z << " " << std::endl;
          glTranslatef(pos.x, pos.y, pos.z);

          glRotatef(rot.y-90, 0, 1, 0);
          glRotatef(-rot.x, 0, 0, 1);
          glRotatef(rot.z, 1, 0, 0);

          glNormalPointer(GL_FLOAT, 0, (*wmo)->normals().data());
          glVertexPointer(3, GL_FLOAT, 0, (*wmo)->vertices().data());
          glDrawElements(GL_TRIANGLES, (*wmo)->indices().size(), GL_UNSIGNED_INT, (*wmo)->indices().data());

          glRotatef(-rot.z, 1, 0, 0);
          glRotatef(rot.x, 0, 0, 1);
          glRotatef(-(rot.y-90), 0, 1, 0);

          glTranslatef(-pos.x, -pos.y, -pos.z);
        }
      }

    }
  }

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  // std::cout << "############# " << p.x << " " << p.y << " " << p.z << std::endl;
  /*glBegin(GL_TRIANGLES);
    for(int i = 0; i < 8*12*64*256; i++) {
      glNormal3f(normales_[i].x, normales_[i].y, normales_[i].z);
      glVertex3f(vertices_[i].x - p.y, vertices_[i].y - p.z, vertices_[i].z - p.x);
    }
    for(AdtList_t::const_iterator adt = adt_list_->begin();
        adt != adt_list_->end();
        ++adt) {
      const glm::vec3 *vertices = (*adt)->vertices();
      const glm::vec3 *normals = (*adt)->normals();
      //std::cout << vertices[0].x - p.x << " " << vertices[0].y - p.y << " " << vertices[0].z - p.z << std::endl;

      for(int i = 0; i < 12*64*256; i++) { //3*4
        glNormal3f(normals[i].x, normals[i].y, normals[i].z);
        glVertex3f(vertices[i].x - p.y, vertices[i].y - p.z, vertices[i].z - p.x);
        glNormal3f(normals[i*12].x, normals[i*12].y, normals[i*12].z);
        glVertex3f(vertices[i*12].x - p.y, vertices[i*12].y - p.z, vertices[i*12].z - p.x);
        glNormal3f(normals[i*12+3].x, normals[i*12+3].y, normals[i*12+3].z);
        glVertex3f(vertices[i*12+3].x - p.y, vertices[i*12+3].y - p.z, vertices[i*12+3].z - p.x);
        glNormal3f(normals[i*12+6].x, normals[i*12+6].y, normals[i*12+6].z);
        glVertex3f(vertices[i*12+6].x - p.y, vertices[i*12+6].y - p.z, vertices[i*12+6].z - p.x);
        glNormal3f(normals[i*12+9].x, normals[i*12+9].y, normals[i*12+9].z);
        glVertex3f(vertices[i*12+9].x - p.y, vertices[i*12+9].y - p.z, vertices[i*12+9].z - p.x);
      }
    }
  glEnd();*/

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glNormalPointer(GL_FLOAT, 0, normales_);
  glVertexPointer(3, GL_FLOAT, 0, vertices_);
  glDrawArrays(GL_TRIANGLES, 0, adt_list_->size() * 12 * 64 * 256);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);


  glutSwapBuffers();
}

void Displayer_c::PassiveMouseCallback(int32_t x, int32_t y) {
  mouse_pos_ = glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

void Displayer_c::MouseCallback(int32_t x, int32_t y) {
  glm::vec2 new_pos(static_cast<float>(x), static_cast<float>(y));
  glm::vec2 distance = new_pos - mouse_pos_;
  camera_.rotation() += (distance * 0.5f);

  glm::mat4 rot4;
  rot4 = glm::rotate(rot4, camera_.rotation().y, glm::vec3(1, 0, 0));
  rot4 = glm::rotate(rot4, camera_.rotation().x, glm::vec3(0, 1, 0));
  glm::mat3 rot(rot4);

  camera_.direction() = (glm::vec3(0, 0, 1) * rot);

  mouse_pos_ = new_pos;
}

void Displayer_c::KeyboardCallback(uint8_t key, int x, int y) {
  glm::vec3 &cam_dir = camera_.direction();
  glm::vec3 &cam_pos = camera_.position();
  const float speed = 20.0f;

  /* key control */
  switch(key) {
    case 'q': {
      camera_.position().y += speed;
      break; }
    case 'e': {
      camera_.position().y -= speed;
      break; }
    case 'a': {
      cam_pos -= glm::vec3(-cam_dir.z, 0, cam_dir.x) * speed;
      break; }
    case 'd': {
      cam_pos += glm::vec3(-cam_dir.z, 0, cam_dir.x) * speed;
      break; }
    case 'w': {
      cam_pos += cam_dir * speed;
      break; }
    case 's': {
      cam_pos -= cam_dir * speed;
      break; }
  }
}