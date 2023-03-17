#include <FL/Fl.H>
#include "window.h"
#include "motion.h"
#include "defmesh.h"
#include "device.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/pinocchioApi.h"
#include "bvh11.hpp"

#include <fstream>
#include <iostream>

//#include "json.hpp"
//void to_json(nlohmann::json& j, const Pinocchio::Vector3& p);

#define SKELETON_RENDER 1
#define SKELETON_CACHE 0

extern const string FILE_CACHE = "skeleton.out";

/*
* struct PinocchioOutput
{
    PinocchioOutput() : attachment(NULL) {}

    vector<Pinocchio::Vector3> embedding;
    Attachment* attachment; //user responsible for deletion
};
*/

int save_PinocchioOutput(Mesh m, Skeleton skeleton, PinocchioOutput o, std::string skelOutName = "skeleton_output.txt", std::string weightOutName = "skinning_weights_output.txt") {
    int i = 0;
    //output skeleton embedding

    //for (i = 0; i < (int)o.embedding.size(); ++i)
    //    o.embedding[i] = (o.embedding[i] - m.toAdd) / m.scale;
    ofstream of_skeleton_normal(skelOutName.c_str());
    for (i = 0; i < (int)o.embedding.size(); ++i) {
        of_skeleton_normal << i << " " << o.embedding[i][0] << " " << o.embedding[i][1] <<
            " " << o.embedding[i][2] << " " << skeleton.fPrev()[i] << endl;
    }

    ofstream of_skeleton_orignal(skelOutName + ".orignal.txt");
    for (i = 0; i < (int)o.embedding.size(); ++i)
        o.embedding[i] = (o.embedding[i] - m.toAdd) / m.scale;
    for (i = 0; i < (int)o.embedding.size(); ++i) {
        of_skeleton_orignal << i << " " << o.embedding[i][0] << " " << o.embedding[i][1] <<
            " " << o.embedding[i][2] << " " << skeleton.fPrev()[i] << endl;
    }

    //output attachment
    std::ofstream of_skinning_weight(weightOutName);

    for (i = 0; i < (int)m.vertices.size(); ++i) {
        Vector<double, -1> v = o.attachment->getWeights(i);
        of_skinning_weight << 0.0; // add zero for root bones skinning weight
        for (int j = 0; j < v.size(); ++j) {
            double d = floor(0.5 + v[j] * 10000.) / 10000.;
            of_skinning_weight << " " << d;
        }
        of_skinning_weight << endl;
    }

    return 0;
}

int load_PinocchioOutput(Mesh m, Skeleton skeleton, PinocchioOutput &o, vector<Vector<double, -1> >& weights, std::string skelOutName = "skeleton.txt", std::string weightOutName = "skinning_weights.txt") {
    int i = 0;

    //output skeleton embedding
    ifstream skeleton_stream(skelOutName);
    int bones = skeleton.fGraph().verts.size() - 1;
    int temp = 0;
    for (i = 0; i < skeleton.fGraph().verts.size(); ++i) {
        skeleton_stream >> temp;
        skeleton_stream >> o.embedding[i][0] >> o.embedding[i][1] >> o.embedding[i][2];
        skeleton_stream >> temp;
    }
    //for (i = 0; i < (int)o.embedding.size(); ++i)
    //    o.embedding[i] = o.embedding[i] * m.scale + m.toAdd;
    ifstream stream(weightOutName);

    if (!stream.is_open()) {
        cout << "Error opening file " << weightOutName << endl;
        return -1;
    }

    int nv = m.vertices.size();
    weights.resize(nv);

    for (i = 0; i < nv; ++i) // initialize the weights vectors so they are big enough
        weights[i][bones - 1] = 0.;
    double root_weight = 0;
    while (!stream.eof()) {
        for (i = 0; i < (int)m.vertices.size(); ++i) {
            stream >> root_weight;
            for (int j = 0; j < bones; ++j) {
                stream >> weights[i][j];
            }
        }
    }

    return 0;
}


void test_bvh11() {
    const std::string bvh_file_path = "../bvh/resources/131_03.bvh";
    // const std::string bvh_file_path = "./data/NBS-dance.bvh";

    // Import data from a BVH file
    bvh11::BvhObject bvh(bvh_file_path);
    // Display basic info
    std::cout << "#Channels       : " << bvh.channels().size() << std::endl;
    std::cout << "#Frames         : " << bvh.frames() << std::endl;
    std::cout << "Frame time      : " << bvh.frame_time() << std::endl;
    std::cout << "Joint hierarchy : " << std::endl;
    bvh.PrintJointHierarchy();

    // Edit the motion data (here, the number of frames is reduced into the half)
    bvh.ResizeFrames(bvh.frames() / 2);

    // Export data to a BVH file
    bvh.WriteBvhFile("./dance.bvh");
}

using namespace bvh11;

void convert_motion_2_bvh11() {
    Device* device = nullptr;
    string motionPath = "data/skeleton.out";
    Motion* motion = new Motion(device, motionPath);
    vector<Pinocchio::Vector3> position = motion->positions[0];
    vector<Rotation> rot = motion->rotations[0];

    unordered_map<int, std::shared_ptr<Joint>> hierarchicalRotations;

    std::shared_ptr<Joint> root_joint = std::make_shared<Joint>("root", nullptr);
    Pinocchio::Vector3 offset = position[0];
    root_joint->offset() = Eigen::Vector3d(offset[0], offset[1], offset[2]);
    hierarchicalRotations.insert({0, root_joint});
    root_joint->channels = { "Xposition", "Yposition", "Zposition", "Xrotation", "Yrotation", "Zrotation" };
    for (int i = 1; i < rot.size(); i++)
    {
        std::shared_ptr<Joint> new_joint = std::make_shared<Joint>(std::to_string(rot[i].joint2), hierarchicalRotations[rot[i].joint1]);
        Pinocchio::Vector3 offset = position[rot[i].joint2] - position[rot[i].joint1];
        //Pinocchio::Vector3 offset = position[rot[i].joint2]; // error
        new_joint->offset() = Eigen::Vector3d(offset[0], offset[1], offset[2]);
        new_joint->has_end_site() = true;
        new_joint->end_site() = Eigen::Vector3d(0, 0, 0);
        new_joint->channels = { "Xrotation", "Yrotation", "Zrotation" };
        hierarchicalRotations.insert({ rot[i].joint2, new_joint });
        hierarchicalRotations[rot[i].joint1]->AddChild(new_joint);
        hierarchicalRotations[rot[i].joint1]->has_end_site() = false;
    }

    Eigen::MatrixXd      motion_;
    motion_.resize(motion->rotations.size(), 3 + 3*motion->rotations[0].size());

    for (int frame_index = 0; frame_index < motion->rotations.size(); ++frame_index)
    {
        auto pos = motion->positions[frame_index];
        motion_(frame_index, 0) = pos[frame_index][0];
        motion_(frame_index, 1) = pos[frame_index][1];
        motion_(frame_index, 2) = pos[frame_index][2];
        for (int channel_index = 0; channel_index < motion->rotations[frame_index].size(); ++channel_index)
        {
            auto r = motion->rotations[frame_index][channel_index].rotation;
            Eigen::Quaterniond quart(-r[3], r[0], r[1], -r[2]);
            Eigen::Vector3d eulerAngle = quart.matrix().eulerAngles(0, 1, 2);
            motion_(frame_index, 3 * channel_index + 3) = eulerAngle[0] * 180 / M_PI;
            motion_(frame_index, 3 * channel_index + 4) = eulerAngle[1] * 180 / M_PI;
            motion_(frame_index, 3 * channel_index + 5) = eulerAngle[2] * 180 / M_PI;
        }
    }
    const std::string bvh_file_path = "../bvh/resources/131_03.bvh";
    // const std::string bvh_file_path = "./data/NBS-dance.bvh";

    // Import data from a BVH file
    bvh11::BvhObject bvh(bvh_file_path);
    // Display basic info
    std::cout << "#Channels       : " << bvh.channels().size() << std::endl;
    std::cout << "#Frames         : " << bvh.frames() << std::endl;
    std::cout << "Frame time      : " << bvh.frame_time() << std::endl;
    std::cout << "Joint hierarchy : " << std::endl;

    // Open the input file
    const std::string file_path = "./data/skeleton_Pinocchio.bvh";
    std::ofstream ofs(file_path);
    assert(ofs.is_open() && "Failed to open the output file.");

    // Hierarch
    ofs << "HIERARCHY"
        << "\n";
    bvh.WriteJointSubHierarchyWithChannels(ofs, root_joint, 0);

    // Motion
    ofs << "MOTION"
        << "\n";
    ofs << "Frames: " << motion->rotations.size() << "\n";
    ofs << "Frame Time: " << 1.0 / 30.0 << "\n";
    // Eigen format
    const Eigen::IOFormat motion_format(Eigen::StreamPrecision, Eigen::DontAlignCols, " ", "", "", "\n", "", "");

    ofs << motion_.format(motion_format);
}

void convert_skeleton_2_bvh11() {
    Device* device = nullptr;
    string motionPath = "data/skeleton.out";
    Motion* motion = new Motion(device, motionPath);
    //vector<Pinocchio::Vector3> position = motion->positions[0];
    vector<Rotation> rot = motion->rotations[0];

    std::string skelOutName = "data/ZiJian_skeleton_output.txt.orignal.txt";
    ifstream skeleton_stream(skelOutName);
    vector<Pinocchio::Vector3> embedding;
    int joints_size = 20;
    embedding.resize(joints_size);
    int temp = 0;
    for (int i = 0; i < joints_size; ++i) {
        skeleton_stream >> temp;
        skeleton_stream >> embedding[i][0] >> embedding[i][1] >> embedding[i][2];
        skeleton_stream >> temp;
    }

    unordered_map<int, std::shared_ptr<Joint>> hierarchicalRotations;

    std::shared_ptr<Joint> root_joint = std::make_shared<Joint>("root", nullptr);
    Pinocchio::Vector3 offset = embedding[0];
    root_joint->offset() = Eigen::Vector3d(offset[0], offset[1], offset[2]);
    hierarchicalRotations.insert({ 0, root_joint });
    root_joint->channels = { "Xposition", "Yposition", "Zposition", "Xrotation", "Yrotation", "Zrotation" };
    for (int i = 1; i < rot.size(); i++)
    {
        std::shared_ptr<Joint> new_joint = std::make_shared<Joint>(std::to_string(rot[i].joint2), hierarchicalRotations[rot[i].joint1]);
        Pinocchio::Vector3 offset = embedding[rot[i].joint2] - embedding[rot[i].joint1];
        //Pinocchio::Vector3 offset = position[rot[i].joint2]; // error
        new_joint->offset() = Eigen::Vector3d(offset[0], offset[1], offset[2]);
        new_joint->has_end_site() = true;
        new_joint->end_site() = Eigen::Vector3d(0, 0, 0);
        new_joint->channels = { "Xrotation", "Yrotation", "Zrotation" };
        hierarchicalRotations.insert({ rot[i].joint2, new_joint });
        hierarchicalRotations[rot[i].joint1]->AddChild(new_joint);
        hierarchicalRotations[rot[i].joint1]->has_end_site() = false;
    }

    const std::string bvh_file_path = "../bvh/resources/131_03.bvh";
    // const std::string bvh_file_path = "./data/NBS-dance.bvh";

    // Import data from a BVH file
    bvh11::BvhObject bvh(bvh_file_path);
    // Open the input file
    const std::string file_path = "./data/ZiJian_skeleton_Pinocchio_orignal.bvh";
    std::ofstream ofs(file_path);
    assert(ofs.is_open() && "Failed to open the output file.");

    // Hierarch
    ofs << "HIERARCHY"
        << "\n";
    bvh.WriteJointSubHierarchyWithChannels(ofs, root_joint, 0);

    // Motion
    ofs << "MOTION"
        << "\n";
    ofs << "Frames: " << 0 << "\n";
    ofs << "Frame Time: " << 1.0 / 30.0 << "\n";
}


int main(int argc, char** argv) {
    if (argc != 6) {
        cout << "Missing either {meshPath} {skeletonPath} {motionPath} {rigged} {animation}" << endl;
        exit(-1);
    }

    const string meshPath = argv[1]; 
    const string skeletonPath = argv[2];
    const string motionPath = argv[3];
    const string rigged = argv[4];
    const string animation = argv[5];

    bool SKELETON_RIGGED = false;
    if (rigged == "rigged") {
        SKELETON_RIGGED = true;
    } else {
        SKELETON_RIGGED = false;
    }

    bool SKELETON_ANIMATION = false;
    if (animation == "live") {
        SKELETON_ANIMATION = true;
    } else {
        SKELETON_ANIMATION = false;
    }

    Debugging::setOutStream(cout);
    //convert_skeleton_2_bvh11();
    //convert_motion_2_bvh11();

    //MyWindow * window = new MyWindow(meshPath);
    Mesh mesh(meshPath);

    // Skeleton skeleton = FileSkeleton(skeletonPath);
    Skeleton skeleton = HumanSkeleton();
    if (!SKELETON_RIGGED) {
        skeleton.scale(0.7);

        mesh.normalizeBoundingBox();
    }
    mesh.writeObj(meshPath + "_normalized.obj");

    Device * device = nullptr;
    if (SKELETON_ANIMATION) {
        device = new Device();
    }

    DeformableMesh * defmesh;
    Motion * motion = new Motion(device, motionPath);

    std::string skelOutName = meshPath + "_skeleton.txt";
    std::string weightOutName = meshPath + "_skinning_weights.txt";

    // if (!SKELETON_RIGGED) {
    if (false) {
        // defmesh = new DeformableMesh(mesh, skeleton, motion);
        PinocchioOutput riggedOut;
        riggedOut.embedding.resize(skeleton.joints.size());
        vector<Vector<double, -1> > skinningWeights;
        load_PinocchioOutput(mesh, skeleton, riggedOut, skinningWeights, skelOutName, weightOutName);
        // TODO: display riggedOut.embedding
        defmesh = new DeformableMesh(mesh, riggedOut.embedding, skinningWeights, motion);
    } else {
        // compute joint skin association and weights to bones attachment
        PinocchioOutput riggedOut = autorig(skeleton, mesh);
        save_PinocchioOutput(mesh, skeleton, riggedOut, skelOutName, weightOutName);
        defmesh = new DeformableMesh(mesh, skeleton, riggedOut.embedding, *(riggedOut.attachment), motion);
    }

    //defmesh->rigged = SKELETON_RIGGED;
    //window->addMesh(defmesh);
    //window->show();

    //return Fl::run();
    return 0;
}