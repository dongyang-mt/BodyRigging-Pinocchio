#include <FL/Fl.H>
#include "window.h"
#include "motion.h"
#include "defmesh.h"
#include "device.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/pinocchioApi.h"
#include "bvh11.hpp"

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
    for (i = 0; i < (int)o.embedding.size(); ++i)
        o.embedding[i] = (o.embedding[i] - m.toAdd) / m.scale;
    ofstream os(skelOutName.c_str());
    for (i = 0; i < (int)o.embedding.size(); ++i) {
        os << i << " " << o.embedding[i][0] << " " << o.embedding[i][1] <<
            " " << o.embedding[i][2] << " " << skeleton.fPrev()[i] << endl;
    }

    //output attachment
    std::ofstream astrm(weightOutName.c_str());

    for (i = 0; i < (int)m.vertices.size(); ++i) {
        Vector<double, -1> v = o.attachment->getWeights(i);
        astrm << 0.0; // add zero for root bones skinning weight
        for (int j = 0; j < v.size(); ++j) {
            double d = floor(0.5 + v[j] * 10000.) / 10000.;
            astrm << " " << d;
        }
        astrm << endl;
    }
    return 0;
}

int load_PinocchioOutput(Mesh m, Skeleton skeleton, PinocchioOutput &o, vector<Vector<double, -1> >& weights, std::string skelOutName = "skeleton_output.txt", std::string weightOutName = "skinning_weights_output.txt") {
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
    for (i = 0; i < (int)o.embedding.size(); ++i)
        o.embedding[i] = o.embedding[i] * m.scale + m.toAdd;
    ifstream stream(weightOutName);

    if (!stream.is_open()) {
        cout << "Error opening file " << weightOutName << endl;
        return -1;
    }

    cout << "Reading " << weightOutName << endl;

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

    //output skinning weight for check
    std::ofstream astrm("data/human_skinning_weight_for_check.txt");

    for (i = 0; i < (int)m.vertices.size(); ++i) {
        Vector<double, -1> v = weights[i];
        astrm << 0.0; // add zero for root bones skinning weight
        for (int j = 0; j < v.size(); ++j) {
            double d = floor(0.5 + v[j] * 10000.) / 10000.;
            astrm << " " << d;
        }
        astrm << endl;
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

    MyWindow * window = new MyWindow(meshPath);
    Mesh mesh(meshPath);

    // Skeleton skeleton = FileSkeleton(skeletonPath);
    Skeleton skeleton = HumanSkeleton();
    if (!SKELETON_RIGGED) {
        skeleton.scale(0.7);

        mesh.normalizeBoundingBox();
    }

    Device * device = nullptr;
    if (SKELETON_ANIMATION) {
        device = new Device();
    }

    DeformableMesh * defmesh;
    Motion * motion = new Motion(device, motionPath);
    vector<vector<Pinocchio::Vector3>> positions = motion->positions;
    vector<vector<Rotation>> rotation = motion->rotations;

    // if (!SKELETON_RIGGED) {
    if (true) {
        // defmesh = new DeformableMesh(mesh, skeleton, motion);
        std::string skelOutName = "data/human_skeleton_output.txt";
        std::string weightOutName = "data/human_skinning_weights_output.txt";
        PinocchioOutput riggedOut;
        riggedOut.embedding.resize(skeleton.joints.size());
        vector<Vector<double, -1> > skinningWeights;
        load_PinocchioOutput(mesh, skeleton, riggedOut, skinningWeights, skelOutName, weightOutName);
        defmesh = new DeformableMesh(mesh, riggedOut.embedding, skinningWeights, motion);
    } else {
        // compute joint skin association and weights to bones attachment
        PinocchioOutput riggedOut = autorig(skeleton, mesh);
        save_PinocchioOutput(mesh, skeleton, riggedOut);
        defmesh = new DeformableMesh(mesh, skeleton, riggedOut.embedding, *(riggedOut.attachment), motion);
    }

    defmesh->rigged = SKELETON_RIGGED;
    window->addMesh(defmesh);
    window->show();

    return Fl::run();
}