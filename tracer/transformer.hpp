#pragma once

#include "includes.hpp"
#include "utils.hpp"
#include "tracer.hpp"

namespace PathTracer {
    class Transformer : public PTObject {
        protected:
        std::vector<glm::dmat4> stack;
        glm::dmat4 current;
        
        public:
            pgl::floatv voffsetAccum;
            pgl::intv flags = 0;
            pgl::intv exflags = 0;
            pgl::floatv4 colormod = pgl::floatv4(1.0f);

            Transformer(){
                current = glm::dmat4(1.0);
            }
        
            glm::mat4 getCurrent(){
                return current;
            }
        
            void reset() {
                stack.resize(0);
                current = glm::dmat4(1.0);
            }

            void multiply(glm::dmat4 mat) {
                current = current * mat;
            }

            void multiply(glm::mat4 mat) {
                current = current * glm::dmat4(mat);
            }

            void multiply(const pgl::floatv * m) {
                multiply(*(glm::mat4 *)m);
            }

            void rotate(pgl::doublev angle, pgl::doublev3 rot){
                current = glm::rotate(current, angle, rot);
            }

            void rotate(pgl::doublev angle, pgl::doublev x, pgl::doublev y, pgl::doublev z) {
                rotate(angle * M_PI / 180.0, pgl::doublev3(x, y, z));
            }
        
            void translate(pgl::doublev3 offset){
                current = glm::translate(current, offset);
            }

            void translate(pgl::doublev x, pgl::doublev y, pgl::doublev z) {
                translate(pgl::doublev3(x, y, z));
            }
        
            void scale(pgl::doublev3 size){
                current = glm::scale(current, size);
            }

            void scale(pgl::doublev x, pgl::doublev y, pgl::doublev z) {
                scale(pgl::doublev3(x, y, z));
            }
        
            void identity(){
                current = glm::dmat4(1.0);
            }
        
            void push(){
                stack.push_back(current);
                //current = new glm::mat4();
            }
        
            void pop(){
                if (stack.size() <= 0) {
                    current = glm::dmat4(1.0);
                }
                else {
                    current = stack[stack.size() - 1];
                }
                stack.pop_back();
            }
    };
}