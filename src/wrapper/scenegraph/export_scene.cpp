/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       GeomPy: Python wrapper for the Plant Graphic Library
 *
 *       Copyright 1995-2003 UMR AMAP 
 *
 *       File author(s): C. Pradal (christophe.pradal@cirad.fr)
 *
 *       $Source$
 *       $Id$
 *
 *       Forum for AMAPmod developers    : amldevlp@cirad.fr
 *
 *  ----------------------------------------------------------------------------
 *
 *                      GNU General Public Licence
 *
 *       This program is free software; you can redistribute it and/or
 *       modify it under the terms of the GNU General Public License as
 *       published by the Free Software Foundation; either version 2 of
 *       the License, or (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public
 *       License along with this program; see the file COPYING. If not,
 *       write to the Free Software Foundation, Inc., 59
 *       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  ----------------------------------------------------------------------------
 */

#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>

#include <scenegraph/scene/scene.h>
#include <scenegraph/scene/shape.h>
#include <scenegraph/geometry/geometry.h>
#include <scenegraph/appearance/appearance.h>
#include <scenegraph/core/action.h>
#include <scenegraph/appearance/material.h>

#include <string>

#include "../util/export_refcountptr.h"
#include "../util/export_property.h"
#include "../util/exception.h"

PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE
using namespace boost::python;
using namespace std;

DEF_POINTEE(Scene)
DEF_POINTEE(SceneObject)

std::string get_sco_name(SceneObject * obj){ return obj->getName(); } 
void set_sco_name(SceneObject * obj, std::string v){ obj->setName(v); } 

void export_SceneObject()
{
  class_< SceneObject,SceneObjectPtr, boost::noncopyable >("SceneObject", no_init)
    .def("getName", &SceneObject::getName, return_value_policy< copy_const_reference >())
    .def("isNamed", &SceneObject::isNamed)
    .def("setName", &SceneObject::setName)
    .add_property("name",get_sco_name,&SceneObject::setName)
    .def("isValid", &SceneObject::isValid)
    .def("apply", &SceneObject::apply)
    .def("copy", &SceneObject::copy)
    .def("getId", &SceneObject::getId)
	.enable_pickling()
    ;
}


ScenePtr sc_fromlist( boost::python::list l ) 
{ 
  ScenePtr scene = new Scene();
  object iter_obj = boost::python::object( handle<>( PyObject_GetIter( l.ptr() ) ) );
  while( 1 )
  {
	    object obj;
		try { 
          obj = iter_obj.attr( "next" )();
		} catch( error_already_set ) { PyErr_Clear(); break; }
		boost::python::extract<GeometryPtr> geom( obj );
		if(geom.check()){
			GeometryPtr g = geom();
			scene->add(Shape(g,Material::DEFAULT_MATERIAL));
		}
		else {
			Shape3DPtr val = boost::python::extract<Shape3DPtr>( obj );
			scene->add( val );
		}
  }
  return scene;
}

Shape3DPtr sc_getitem( Scene* s, size_t pos )
{
  if( pos < 0 && pos > -s->getSize() ) return s->getAt( s->getSize() + pos );
  if (pos < s->getSize()) return s->getAt( pos );
  else throw PythonExc_IndexError();
}

ShapePtr sc_find( Scene* s, size_t id )
{
  ShapePtr res = s->getShapeId( id );
  if (res) return res;
  else throw PythonExc_IndexError();
}

void sc_setitem( Scene* s, size_t pos, Shape3DPtr v )
{
  if( pos < 0 && pos > -s->getSize() ) return s->setAt( s->getSize() + pos, v );
  if (pos < s->getSize()) s->setAt( pos ,v );
  else throw PythonExc_IndexError();
}

void sc_delitem( Scene* s, size_t pos )
{
  Scene::iterator it;
  if( pos < 0 && pos > -s->getSize() ) { it = s->getEnd()+pos;  return s->remove( it ); }
  if (pos < s->getSize()) { it = s->getBegin() + pos; s->remove(it ); } 
  else throw PythonExc_IndexError();
}

ScenePtr sc_iadd(Scene* s ,Shape sh){
  s->add(sh);
  return s;
}

ScenePtr sc_iadd2(Scene* s ,Shape3DPtr sh){
  s->add(sh);
  return s;
}

ScenePtr sc_iadd3(Scene* s ,Scene* s2){
  s->merge(s2);
  return s;
}

ScenePtr sc_iadd4(Scene* s ,GeometryPtr sh){
	s->add(Shape(sh,Material::DEFAULT_MATERIAL));
	return s;
}

ScenePtr sc_add(Scene* s ,Scene* s2){
  ScenePtr s3 = ScenePtr(new Scene(*s));
  s3->merge(s2);
  return s3;
}
void sc_read(Scene* s ,const std::string& fname){
	s->read(fname);
}

void sc_save(Scene* s ,const std::string& fname){
	s->save(fname);
}

uint32_t sc_index( Scene* sc, Shape3DPtr sh)
{
  sc->lock();
  Scene::iterator it = std::find(sc->getBegin(),sc->getEnd(),sh);
  if (it ==  sc->getEnd())
	{sc->unlock(); throw PythonExc_ValueError(); }
  uint32_t dist = std::distance(sc->getBegin(),it);
  sc->unlock();
  return dist;
}

void sc_remove( Scene* sc, Shape3DPtr sh)
{
  sc->lock();
  Scene::iterator it = std::find(sc->getBegin(),sc->getEnd(),sh);
  if (it ==  sc->getEnd())
	{sc->unlock(); throw PythonExc_ValueError(); }
  sc->remove(it);
  sc->unlock();
}

void export_Scene()
{
  class_<Scene,ScenePtr, boost::noncopyable>("Scene",init<const std::string&>("Scene() -> Create an empty scene"))
    .def(init< optional< unsigned int > >())
    .def(init< const Scene& >())
    .def( "__init__", make_constructor( sc_fromlist ) ) 
	.def("__iadd__", &sc_iadd)
	.def("__iadd__", &sc_iadd2)
	.def("__iadd__", &sc_iadd3)
	.def("__iadd__", &sc_iadd4)
	.def("__add__", &sc_add)
    .def("add", (void (Scene::*)(const Shape &) ) &Scene::add )
    .def("add", (void (Scene::*)(const RefCountPtr<Shape3D> &) )&Scene::add)
    .def("add", &Scene::merge)
    .def("merge", &Scene::merge)
    .def("__len__", &Scene::getSize)
    .def("__getitem__", &sc_getitem)
    .def("__setitem__", &sc_setitem)
    .def("__delitem__", &sc_delitem)
    .def("clear", &Scene::clear)
    .def("merge", &Scene::merge)
    .def("find", &sc_find)
    .def("index", &sc_index)
    .def("remove", &sc_remove)
    .def("isValid", (bool (Scene::*)() const)&Scene::isValid)
    .def("apply", &Scene::apply)
    .def("applyGeometryFirst", &Scene::applyGeometryFirst)
    .def("applyGeometryOnly", &Scene::applyGeometryOnly)
    .def("applyAppearanceFirst", &Scene::applyAppearanceFirst)
    .def("applyAppearanceOnly", &Scene::applyAppearanceOnly)
    .def("copy", &Scene::copy)
    .def("read", &sc_read)
    .def("save", &sc_save)
  	.enable_pickling()
  ;

}


