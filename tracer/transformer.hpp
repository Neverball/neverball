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
            pgl::floatv voffsetAccum;
            pgl::intv flags = 0;

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

            void multiply(const pgl::floatv * m) {
                multiply(*(glm::mat4 *)m);
            }

            void rotate(pgl::floatv angle, pgl::floatv3 rot){
                *current = glm::rotate(*current, angle, rot);
            }

            void rotate(pgl::floatv angle, pgl::floatv x, pgl::floatv y, pgl::floatv z) {
                rotate(angle * M_PI / 180.0f, pgl::floatv3(x, y, z));
            }
        
            void translate(pgl::floatv3 offset){
                *current = glm::translate(*current, offset);
            }

            void translate(pgl::floatv x, pgl::floatv y, pgl::floatv z) {
                translate(pgl::floatv3(x, y, z));
            }
        
            void scale(pgl::floatv3 size){
                *current = glm::scale(*current, size);
            }

            void scale(pgl::floatv x, pgl::floatv y, pgl::floatv z) {
                scale(pgl::floatv3(x, y, z));
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