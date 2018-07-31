//
//  ofxSloppyAppManager.h
//
//  Created by yutaAsai on 2018/06/29.
//
//

#ifndef ofxSloppyAppManager_h
#define ofxSloppyAppManager_h
#include <type_traits>
#include <typeinfo>
#include <cxxabi.h>
#include <string>
#include <stdlib.h>
#include <map>
#include <string>

namespace aaa{
    namespace ofx{
        namespace sloppy_app_manager{
            namespace detail{
                namespace utils{
                    namespace rtti{
                        template<typename T>
                        static const std::string get_name(const T& obj){
                            const std::type_info& id = typeid(obj);
                            int stat(0);
                            std::string name_buffer;
                            char *name = abi::__cxa_demangle(id.name(),0,0,&stat);
                            if(name!=NULL) {
                                if(stat == 0) {
                                    name_buffer = name;
                                    free(name);
                                    return name_buffer;
                                }
                                free(name);
                            }
                            return "";
                        }
                    };
                    
                    template<class...>
                    using void_t = void;
                    template<class AlwaysVoid, template<class...>class Op, class ...Args>
                    struct is_detected_impl :std::false_type{};
                    template<template<class...>class Op, class ...Args>
                    struct is_detected_impl<void_t<Op<Args...>>, Op, Args...> :std::true_type{};
                };
                template<template<class...>class Op, class ...Args>
                using is_detected = utils::is_detected_impl<void, Op, Args...>;
                template<class T> using has_setup_op = decltype(std::declval<T>().setup());
                template<class T> using has_setup = is_detected<has_setup_op, T>;
                template<class T> using has_update_op = decltype(std::declval<T>().update());
                template<class T> using has_update = is_detected<has_update_op, T>;
                template<class T> using has_draw_op = decltype(std::declval<T>().draw());
                template<class T> using has_draw = is_detected<has_draw_op, T>;
                template<class T> using has_alpha_op = decltype(std::declval<T>().alpha);
                template<class T> using has_alpha = is_detected<has_alpha_op, T>;

            };
            
            template<typename T>
            struct value_setter{
                value_setter()
                : t(nullptr)
                {}
            
                value_setter(T& t)
                : t(&t)
                {}
                
                value_setter(const value_setter<T>& vs)
                : t(vs.t)
                {}
                
                value_setter(value_setter<T>&& vs)
                : t(vs.t)
                {}
                
                void operator()(const T val){ if(t) *t = val; }
                T* t;
            };
            
            struct app_base{
                virtual ~app_base(){}
                virtual void setup() = 0;
                virtual void update() = 0;
                virtual void draw() = 0;
                virtual void set_alpha(const float alpha) = 0;
            };
            
            template<typename APP_TYPE>
            struct app_holder : app_base{
                void setup(){
                    call_setup();
                }
                void update(){
                    call_update();
                }
                void draw(){
                    call_draw();
                }
                void set_alpha(const float alpha){
                    call_set_alpha(alpha);
                }
            private:
                
                template <typename T = APP_TYPE>
                auto call_setup()
                -> typename std::enable_if<detail::has_setup<T>::value, void>::type
                {
                    app.setup();
                }
                
                template <typename T = APP_TYPE>
                auto call_setup()
                -> typename std::enable_if<!detail::has_setup<T>::value, void>::type
                {}
                
                template <typename T = APP_TYPE>
                auto call_update()
                -> typename std::enable_if<detail::has_update<T>::value, void>::type
                {
                    app.update();
                }
                
                template <typename T = APP_TYPE>
                auto call_update()
                -> typename std::enable_if<!detail::has_update<T>::value, void>::type
                {}
                
                template <typename T = APP_TYPE>
                auto call_draw()
                -> typename std::enable_if<detail::has_draw<T>::value, void>::type
                {
                    app.draw();
                }
                
                template <typename T = APP_TYPE>
                auto call_draw()
                -> typename std::enable_if<!detail::has_draw<T>::value, void>::type
                {}
                
                template <typename T = APP_TYPE>
                auto call_set_alpha(const float alpha)
                -> typename std::enable_if<detail::has_alpha<T>::value, void>::type
                {
                    app.alpha = alpha;
                }
                
                template <typename T = APP_TYPE>
                auto call_set_alpha(const float alpha)
                -> typename std::enable_if<!detail::has_alpha<T>::value, void>::type
                {}
                
                APP_TYPE app;
            };
            
            struct manager{
                template<typename APP_TYPE>
                void set(const float alpha = 1.f){
                    const std::string name = detail::utils::rtti::get_name(APP_TYPE());
                    app_map[name] = std::pair<float, std::shared_ptr<app_base>>(alpha,std::shared_ptr<app_base>( new app_holder<APP_TYPE>));
                    app_map.at(name).second -> setup();
                    app_map.at(name).second -> set_alpha(alpha);
                    names.push_back(name);
                }
                
                void update(){
                    for(const auto& e : names){
                        if(app_map.count(e)){
                            if(app_map.at(e).first > 0.f){
                                app_map.at(e).second -> set_alpha(app_map.at(e).first);
                                app_map.at(e).second -> update();
                            }
                        }
                    }
                }
                
                void draw(){
                    for(const auto& e : names){
                        if(app_map.count(e)){
                            if(app_map.at(e).first > 0.f){
                                app_map.at(e).second -> draw();
                            }
                        }
                    }
                }
                
                value_setter<float> operator[](std::string name){
                    if(app_map.count(name)) return std::move(value_setter<float>(app_map.at(name).first));
                    else return std::move(value_setter<float>());
                }
                
            private:
                std::map<std::string, std::pair<float ,std::shared_ptr<app_base>>> app_map;
                std::vector<std::string> names;
            };
        };
    };
};

using ofxSloppyAppManager = aaa::ofx::sloppy_app_manager::manager;

#endif /* ofxSloppyAppManager_h */
