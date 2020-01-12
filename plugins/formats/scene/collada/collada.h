/*************************************************************************/
/*  collada.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include "core/io/xml_parser.h"
#include "core/map.h"
#include "core/math/transform.h"
#include "core/math/vector2.h"
#include "core/project_settings.h"
#include "scene/resources/material.h"

class Collada {
public:
    enum ImportFlags {
        IMPORT_FLAG_SCENE = 1,
        IMPORT_FLAG_ANIMATION = 2
    };

    struct Image {

        se_string path;
    };

    struct Material {

        se_string name;
        se_string instance_effect;
    };

    struct Effect {

        se_string name;
        Map<se_string, Variant> params;

        struct Channel {

            int uv_idx;
            se_string texture;
            Color color;
            Channel() { uv_idx = 0; }
        };

        Channel diffuse, specular, emission, bump;
        float shininess;
        bool found_double_sided;
        bool double_sided;
        bool unshaded;

        se_string get_texture_path(const se_string &p_source, Collada &state) const;

        Effect() {
            diffuse.color = Color(1, 1, 1, 1);
            double_sided = true;
            found_double_sided = false;
            shininess = 40;
            unshaded = false;
        }
    };

    struct CameraData {

        enum Mode {
            MODE_PERSPECTIVE,
            MODE_ORTHOGONAL
        };

        Mode mode;

        union {
            struct {
                float x_fov;
                float y_fov;
            } perspective;
            struct {
                float x_mag;
                float y_mag;
            } orthogonal;
        };

        float aspect;
        float z_near;
        float z_far;

        CameraData() :
                mode(MODE_PERSPECTIVE),
                aspect(1),
                z_near(0.1f),
                z_far(100) {
            perspective.x_fov = 0;
            perspective.y_fov = 0;
        }
    };

    struct LightData {

        enum Mode {
            MODE_AMBIENT,
            MODE_DIRECTIONAL,
            MODE_OMNI,
            MODE_SPOT
        };

        Mode mode;

        Color color;

        float constant_att;
        float linear_att;
        float quad_att;

        float spot_angle;
        float spot_exp;

        LightData() :
                mode(MODE_AMBIENT),
                color(Color(1, 1, 1, 1)),
                constant_att(0),
                linear_att(0),
                quad_att(0),
                spot_angle(45),
                spot_exp(1) {
        }
    };

    struct MeshData {

        se_string name;
        struct Source {

            PODVector<float> array;
            int stride;
        };

        Map<se_string, Source> sources;

        struct Vertices {

            Map<se_string, se_string> sources;
        };

        Map<se_string, Vertices> vertices;

        struct Primitives {

            struct SourceRef {

                se_string source;
                int offset;
            };

            se_string material;
            Map<se_string, SourceRef> sources;
            PODVector<float> polygons;
            PODVector<float> indices;
            int count;
            int vertex_size;
        };

        Vector<Primitives> primitives;

        bool found_double_sided;
        bool double_sided;

        MeshData() {
            found_double_sided = false;
            double_sided = true;
        }
    };

    struct CurveData {

        se_string name;
        bool closed;

        struct Source {

            Vector<se_string> sarray;
            PODVector<float> array;
            int stride;
        };

        Map<se_string, Source> sources;

        Map<se_string, se_string> control_vertices;

        CurveData() {

            closed = false;
        }
    };
    struct SkinControllerData {

        se_string base;
        bool use_idrefs;

        Transform bind_shape;

        struct Source {

            Vector<se_string> sarray; //maybe for names
            PODVector<float> array;
            int stride = 1;
        };

        Map<se_string, Source> sources;

        struct Joints {

            Map<se_string, se_string> sources;
        } joints;

        struct Weights {

            struct SourceRef {

                se_string source;
                int offset;
            };

            se_string material;
            Map<se_string, SourceRef> sources;
            PODVector<float> sets;
            PODVector<float> indices;
            int count;
        } weights;

        Map<se_string, Transform> bone_rest_map;

        SkinControllerData() { use_idrefs = false; }
    };

    struct MorphControllerData {

        se_string mesh;
        se_string mode;

        struct Source {

            int stride;
            Vector<se_string> sarray; //maybe for names
            PODVector<float> array;
            Source() { stride = 1; }
        };

        Map<se_string, Source> sources;

        Map<se_string, se_string> targets;
        MorphControllerData() {}
    };

    struct Vertex {

        int idx;
        Vector3 vertex;
        Vector3 normal;
        Vector3 uv;
        Vector3 uv2;
        Plane tangent;
        Color color;
        int uid;
        struct Weight {
            int bone_idx;
            float weight;
            bool operator<(const Weight w) const { return weight > w.weight; } //heaviest first
        };

        Vector<Weight> weights;

        void fix_weights() {

            weights.sort();
            if (weights.size() > 4) {
                //cap to 4 and make weights add up 1
                weights.resize(4);
                float total = 0;
                for (int i = 0; i < 4; i++)
                    total += weights[i].weight;
                if (total)
                    for (int i = 0; i < 4; i++)
                        weights.write[i].weight /= total;
            }
        }

        void fix_unit_scale(Collada &state);

        bool operator<(const Vertex &p_vert) const {

            if (uid == p_vert.uid) {
                if (vertex == p_vert.vertex) {
                    if (normal == p_vert.normal) {
                        if (uv == p_vert.uv) {
                            if (uv2 == p_vert.uv2) {

                                if (!weights.empty() || !p_vert.weights.empty()) {

                                    if (weights.size() == p_vert.weights.size()) {

                                        for (int i = 0; i < weights.size(); i++) {
                                            if (weights[i].bone_idx != p_vert.weights[i].bone_idx)
                                                return weights[i].bone_idx < p_vert.weights[i].bone_idx;

                                            if (weights[i].weight != p_vert.weights[i].weight)
                                                return weights[i].weight < p_vert.weights[i].weight;
                                        }
                                    } else {
                                        return weights.size() < p_vert.weights.size();
                                    }
                                }

                                return (color < p_vert.color);
                            } else
                                return (uv2 < p_vert.uv2);
                        } else
                            return (uv < p_vert.uv);
                    } else
                        return (normal < p_vert.normal);
                } else
                    return vertex < p_vert.vertex;
            } else
                return uid < p_vert.uid;
        }

        Vertex() {
            uid = 0;
            idx = 0;
        }
    };
    struct Node {

        enum Type {

            TYPE_NODE,
            TYPE_JOINT,
            TYPE_SKELETON, //this bone is not collada, it's added afterwards as optimization
            TYPE_LIGHT,
            TYPE_CAMERA,
            TYPE_GEOMETRY
        };

        struct XForm {

            enum Op {
                OP_ROTATE,
                OP_SCALE,
                OP_TRANSLATE,
                OP_MATRIX,
                OP_VISIBILITY
            };

            se_string id;
            Op op;
            PODVector<float> data;
        };

        Type type;

        se_string name;
        se_string id;
        se_string empty_draw_type;
        bool noname;
        Vector<XForm> xform_list;
        Transform default_transform;
        Transform post_transform;
        Vector<Node *> children;

        Node *parent;

        Transform compute_transform(Collada &state) const;
        Transform get_global_transform() const;
        Transform get_transform() const;

        bool ignore_anim;

        Node() {
            noname = false;
            type = TYPE_NODE;
            parent = nullptr;
            ignore_anim = false;
        }
        virtual ~Node() {
            for (int i = 0; i < children.size(); i++)
                memdelete(children[i]);
        };
    };

    struct NodeSkeleton : public Node {

        NodeSkeleton() { type = TYPE_SKELETON; }
    };

    struct NodeJoint : public Node {

        NodeSkeleton *owner;
        se_string sid;
        NodeJoint() {
            type = TYPE_JOINT;
            owner = nullptr;
        }
    };

    struct NodeGeometry : public Node {

        bool controller;
        se_string source;

        struct Material {
            se_string target;
        };

        Map<se_string, Material> material_map;
        Vector<se_string> skeletons;

        NodeGeometry() { type = TYPE_GEOMETRY; }
    };

    struct NodeCamera : public Node {

        se_string camera;

        NodeCamera() { type = TYPE_CAMERA; }
    };

    struct NodeLight : public Node {

        se_string light;

        NodeLight() { type = TYPE_LIGHT; }
    };

    struct VisualScene {

        se_string name;
        Vector<Node *> root_nodes;

        ~VisualScene() {
            for (int i = 0; i < root_nodes.size(); i++)
                memdelete(root_nodes[i]);
        }
    };

    struct AnimationClip {

        se_string name;
        float begin;
        float end;
        Vector<se_string> tracks;

        AnimationClip() {
            begin = 0;
            end = 1;
        }
    };

    struct AnimationTrack {

        se_string id;
        se_string target;
        se_string param;
        se_string component;
        bool property;

        enum InterpolationType {
            INTERP_LINEAR,
            INTERP_BEZIER
        };

        struct Key {

            enum Type {
                TYPE_FLOAT,
                TYPE_MATRIX
            };

            float time;
            PODVector<float> data;
            Point2 in_tangent;
            Point2 out_tangent;
            InterpolationType interp_type;

            Key() { interp_type = INTERP_LINEAR; }
        };

        PODVector<float> get_value_at_time(float p_time) const;

        Vector<Key> keys;

        AnimationTrack() { property = false; }
    };

    /****************/
    /* IMPORT STATE */
    /****************/

    struct State {

        int import_flags;

        float unit_scale;
        Vector3::Axis up_axis;
        bool z_up;

        struct Version {

            int major, minor, rev;

            bool operator<(const Version &p_ver) const { return (major == p_ver.major) ? ((minor == p_ver.minor) ? (rev < p_ver.rev) : minor < p_ver.minor) : major < p_ver.major; }
            Version(int p_major = 0, int p_minor = 0, int p_rev = 0) {
                major = p_major;
                minor = p_minor;
                rev = p_rev;
            }
        } version;

        Map<se_string, CameraData> camera_data_map;
        Map<se_string, MeshData> mesh_data_map;
        Map<se_string, LightData> light_data_map;
        Map<se_string, CurveData> curve_data_map;

        Map<se_string, se_string> mesh_name_map;
        Map<se_string, se_string> morph_name_map;
        Map<se_string, se_string> morph_ownership_map;
        Map<se_string, SkinControllerData> skin_controller_data_map;
        Map<se_string, MorphControllerData> morph_controller_data_map;

        Map<se_string, Image> image_map;
        Map<se_string, Material> material_map;
        Map<se_string, Effect> effect_map;

        Map<se_string, VisualScene> visual_scene_map;
        Map<se_string, Node *> scene_map;
        Set<se_string> idref_joints;
        Map<se_string, se_string> sid_to_node_map;
        //Map<se_string,NodeJoint*> bone_map;

        Map<se_string, Transform> bone_rest_map;

        se_string local_path;
        se_string root_visual_scene;
        se_string root_physics_scene;

        Vector<AnimationClip> animation_clips;
        Vector<AnimationTrack> animation_tracks;
        Map<se_string, Vector<int> > referenced_tracks;
        Map<se_string, Vector<int> > by_id_tracks;

        float animation_length;

        State() :
                import_flags(0),
                unit_scale(1.0),
                up_axis(Vector3::AXIS_Y),
                animation_length(0) {
        }
    } state;

    Error load(se_string_view p_path, int p_flags = 0);

    Collada();

    Transform fix_transform(const Transform &p_transform);

    Transform get_root_transform() const;

    int get_uv_channel(se_string p_name);

private: // private stuff
    Map<se_string, int> channel_map;

    void _parse_asset(XMLParser &parser);
    void _parse_image(XMLParser &parser);
    void _parse_material(XMLParser &parser);
    void _parse_effect_material(XMLParser &parser, Effect &effect, se_string &id);
    void _parse_effect(XMLParser &parser);
    void _parse_camera(XMLParser &parser);
    void _parse_light(XMLParser &parser);
    void _parse_animation_clip(XMLParser &parser);

    void _parse_mesh_geometry(XMLParser &parser, se_string p_id, se_string p_name);
    void _parse_curve_geometry(XMLParser &parser, se_string p_id, se_string p_name);

    void _parse_skin_controller(XMLParser &parser, se_string p_id);
    void _parse_morph_controller(XMLParser &parser, se_string p_id);
    void _parse_controller(XMLParser &parser);

    Node *_parse_visual_instance_geometry(XMLParser &parser);
    Node *_parse_visual_instance_camera(XMLParser &parser);
    Node *_parse_visual_instance_light(XMLParser &parser);

    Node *_parse_visual_node_instance_data(XMLParser &parser);
    Node *_parse_visual_scene_node(XMLParser &parser);
    void _parse_visual_scene(XMLParser &parser);

    void _parse_animation(XMLParser &parser);
    void _parse_scene(XMLParser &parser);
    void _parse_library(XMLParser &parser);

    Variant _parse_param(XMLParser &parser);
    PODVector<float> _read_float_array(XMLParser &parser);
    Vector<se_string> _read_string_array(XMLParser &parser);
    Transform _read_transform(XMLParser &parser);
    se_string _read_empty_draw_type(XMLParser &parser);

    void _joint_set_owner(Collada::Node *p_node, NodeSkeleton *p_owner);
    void _create_skeletons(Collada::Node **p_node, NodeSkeleton *p_skeleton = nullptr);
    void _find_morph_nodes(VisualScene *p_vscene, Node *p_node);
    bool _remove_node(Node *p_parent, Node *p_node);
    void _remove_node(VisualScene *p_vscene, Node *p_node);
    void _merge_skeletons2(VisualScene *p_vscene);
    void _merge_skeletons(VisualScene *p_vscene, Node *p_node);
    bool _optimize_skeletons(VisualScene *p_vscene, Node *p_node);

    bool _move_geometry_to_skeletons(VisualScene *p_vscene, Node *p_node, ListPOD<Node *> *p_mgeom);

    void _optimize();
};

