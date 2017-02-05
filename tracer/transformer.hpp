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
            stack.push_back(glm::mat4());
            current = &stack[stack.size()-1];
        }
        
        glm::mat4 getCurrent(){
            return *current;
        }
        
        void reset() {
            stack.resize(0);
            stack.push_back(glm::mat4());
            current = &stack[stack.size() - 1];
        }

        void multiply(glm::mat4 mat) {
            *current = (*current) * mat;
        }

        void multiply(float * m) {
            multiply(*(glm::mat4 *)m);
        }

        void rotate(float angle, glm::vec3 rot){
            *current = glm::rotate(*current, angle, rot);
        }

        void scale(float angle, float x, float y, float z) {
            rotate(angle * 180.0f / M_PI, glm::vec3(x, y, z));
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
            *current = glm::mat4();
        }
        
        void push(){
            stack.push_back(glm::mat4());
            current = &stack[stack.size() - 1];
        }
        
        void pop(){
            stack.pop_back();
            if (stack.size() <= 0) this->push();
            current = &stack[stack.size() - 1];
        }
    };
}