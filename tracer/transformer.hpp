#pragma once

#include "includes.hpp"
#include "utils.hpp"
#include "tracer.hpp"

namespace PathTracer {
    class Transformer : public PTObject {
        protected:
        std::vector<glm::mat4> stack;
        glm::mat4 * current;
        
        public:
        Transformer(){
            current = new glm::mat4(1.0f);
        }
        
        glm::mat4 getCurrent(){
            return *current;
        }
        
        void reset() {
            stack.resize(0);
            *current = glm::mat4(1.0f);
        }

        void multiply(glm::mat4 mat) {
            *current = (*current) * mat;
        }

        void multiply(const float * m) {
            multiply(*(glm::mat4 *)m);
        }

        void rotate(float angle, glm::vec3 rot){
            *current = glm::rotate(*current, angle, rot);
        }

        void rotate(float angle, float x, float y, float z) {
            rotate(angle * M_PI / 180.0f, glm::vec3(x, y, z));
        }
        
        void translate(glm::vec3 offset){
            *current = glm::translate(*current, offset);
        }

        void translate(float x, float y, float z) {
            translate(glm::vec3(x, y, z));
        }
        
        void scale(glm::vec3 size){
            *current = glm::scale(*current, size);
        }

        void scale(float x, float y, float z) {
            scale(glm::vec3(x, y, z));
        }
        
        void identity(){
            *current = glm::mat4(1.0f);
        }
        
        void push(){
            stack.push_back(*current);
            //current = new glm::mat4();
        }
        
        void pop(){
            if (stack.size() <= 0) {
                *current = glm::mat4(1.0f);
            }
            else {
                *current = stack[stack.size() - 1];
            }
            stack.pop_back();
        }
    };
}