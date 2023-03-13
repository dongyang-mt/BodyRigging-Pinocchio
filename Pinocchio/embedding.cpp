/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "pinocchioApi.h"
#include "debugging.h"

#include <fstream>

#include "json.hpp"
using json = nlohmann::json;

//void to_json(json& j, const Pinocchio::Vector3& p);
//void from_json(const json& j, Pinocchio::Vector3& p);
//void to_json(json& j, const Sphere& p);

void to_json(json& j, const Pinocchio::Vector3& p) {
    j = json::array({ p[0], p[1], p[2] });
}

void to_json(json& j, const PtGraph& p) {
    j = json{ {"verts", p.verts}, {"edges", p.edges} };
}

void from_json(const json& j, Pinocchio::Vector3& p) {
    j[0].get_to(p[0]);
    j[1].get_to(p[1]);
    j[2].get_to(p[2]);
}
void to_json(json& j, const Sphere& p) {
    j = json{ {"radius", p.radius}, {"center", p.center} };
}

struct FP //information for penalty functions
{
    FP(const PtGraph &inG, const Skeleton &inSk, const vector<Sphere> &inS)
        : graph(inG), given(inSk), sph(inS), paths(inG) {}

    const PtGraph &graph;
    const Skeleton &given;
    const vector<Sphere> &sph;
    AllShortestPather paths;
    double footBase;
};

struct PartialMatch
{
    PartialMatch(int vsz) : penalty(0), heuristic(0) { vTaken.resize(vsz, false); }
    
    vector<int> match;
    double penalty;
    double heuristic;
    bool operator<(const PartialMatch &pm) const { return heuristic > pm.heuristic; } //smallest penalty first
    
    vector<bool> vTaken;
};

class PenaltyFunction
{
public:
    PenaltyFunction(FP *inFp) : fp(inFp), weight(0.01) {}
    virtual ~PenaltyFunction() {}

    virtual double get(const PartialMatch &cur, int next, int idx) const = 0;

    FP *fp;
    double weight;
};

vector<vector<int> > computePossibilities(const PtGraph &graph, const vector<Sphere> &spheres,
                                          const Skeleton &skeleton)
{
    int i, j;

    vector<vector<int> > out(skeleton.cGraph().verts.size());
    vector<int> allVerts, limbVerts, fatVerts;
    for(i = 0; i < (int)graph.verts.size(); ++i) {
        allVerts.push_back(i);
        const vector<int> &edg = graph.edges[i];
        double rad = spheres[i].radius;
        Pinocchio::Vector3 cur = graph.verts[i];
        for(j = 0; j < (int)edg.size(); ++j) {
            Pinocchio::Vector3 e1 = graph.verts[edg[j]];
            int k;
            for(k = 0; k < (int)edg.size(); ++k) {
                if(rad > 2. * spheres[edg[k]].radius)
                    continue;
                Pinocchio::Vector3 e2 = graph.verts[edg[k]];
                if((e2 - cur).normalize() * (cur - e1).normalize() > 0.8)
                    break;
            }
            if(k < (int)edg.size())
                continue;
            limbVerts.push_back(i);
            break;
        }
    }
    vector<double> rads;
    for(i = 0; i < (int)graph.verts.size(); ++i)
        rads.push_back(spheres[i].radius);
    sort(rads.begin(), rads.end());
    double cutoff = (int)rads.size() < 50 ? 0. : rads[rads.size() - 50];
    for(i = 0; i < (int)graph.verts.size(); ++i)
        if(spheres[i].radius >= cutoff)
            fatVerts.push_back(i);
    Debugging::out() << "Extrem, fat verts " << limbVerts.size() << " " << fatVerts.size() << endl;
    
    for(i = 0; i < (int)out.size(); ++i) {
        bool limb = (skeleton.cGraph().edges[i].size() == 1);
        bool fat = skeleton.cFat()[i];

        if(fat)
            out[i] = fatVerts;
        else if(limb)
            out[i] = limbVerts;
        else
            out[i] = allVerts;
    }

    return out;
}

vector<PenaltyFunction *> getPenaltyFunctions(FP *fp); //user responsible for deletion of penalties

double computePenalty(const vector<PenaltyFunction *> &penaltyFunctions,
                      const PartialMatch &cur, int next, int idx = -1)
{
    if(idx == -1)
        idx = cur.match.size();
    if(idx == 0)
        return 0;

    double out = 0.;
    for(int i = 0; i < (int)penaltyFunctions.size(); ++i) {
        double penalty = penaltyFunctions[i]->get(cur, next, idx) * penaltyFunctions[i]->weight;
        if(penalty > 1.)
            return 2.;
        out += penalty;
    }
    return out;
}

vector<int> discreteEmbed(const PtGraph &graph, const vector<Sphere> &spheres,
                          const Skeleton &skeleton, const vector<vector<int> > &possibilities)
{
    json json_graph = graph;
    std::ofstream file_graph("graph_distanceField_spheres.json");
    file_graph << std::setw(4) << json_graph << std::endl;
    //std::cout << json_graph.dump(4) << std::endl;

    json json_spheres = spheres;
    std::ofstream file_spheres("spheres.json");
    file_spheres << std::setw(4) << json_spheres << std::endl;
    //std::cout << json_spheres.dump(4) << std::endl;
    
    json json_skeleton;
    json_skeleton["fGraphV"] = skeleton.fGraph();
    json_skeleton["fPrev"] = skeleton.fPrev();
    json_skeleton["fSym"] = skeleton.fSym();

    json_skeleton["cGraph"] = skeleton.cGraph();
    json_skeleton["cPrev"] = skeleton.cPrev();
    json_skeleton["cSym"] = skeleton.cSym();
    json_skeleton["cFeet"] = skeleton.cFeet();
    json_skeleton["cFat"] = skeleton.cFat();

    json_skeleton["joints_names"] = skeleton.fGraph();
    std::ofstream file_skeleton("skeleton.json");
    file_skeleton << std::setw(4) << json_skeleton << std::endl;
    //std::cout << json_skeleton.dump(4) << std::endl;

    json json_possibilities = possibilities;
    std::ofstream file_possibilities("possibilities.json");
    file_possibilities << std::setw(4) << json_graph << std::endl;
    //std::cout << json_possibilities.dump(4) << std::endl;

    int i, j;
    FP fp(graph, skeleton, spheres);

    fp.footBase = 1.;
    for(i = 0; i < (int)graph.verts.size(); ++i)
        fp.footBase = min(fp.footBase, graph.verts[i][1]);

    vector<PenaltyFunction *> penaltyFunctions = getPenaltyFunctions(&fp);

    int toMatch = skeleton.cGraph().verts.size();
    
    Debugging::out() << "Matching!" << endl;
    
    priority_queue<PartialMatch> todo;
    
    PartialMatch output(graph.verts.size());
    todo.push(output);
    
    int maxSz = 0;
    
    while(!todo.empty()) {
        PartialMatch cur = todo.top();
        todo.pop();
        
        int idx = cur.match.size();
        
        int curSz = (int)log((double)todo.size());
        if(curSz > maxSz) {
            maxSz = curSz;
            if(maxSz > 3)
                Debugging::out() << "Reached " << todo.size() << endl;
        }
        
        if(idx == toMatch) {
            output = cur;
            Debugging::out() << "Found: residual = " << cur.penalty << endl;
            break;
        }
        
        for(i = 0; i < (int)possibilities[idx].size(); ++i) {
            int candidate = possibilities[idx][i];
            int k;
            double extraPenalty = computePenalty(penaltyFunctions, cur, candidate);

            if(extraPenalty < 0)
                Debugging::out() << "ERR = " << extraPenalty << endl;
            if(cur.penalty + extraPenalty < 1.) {
                PartialMatch next = cur;
                next.match.push_back(candidate);
                next.penalty += extraPenalty;
                next.heuristic = next.penalty;
                
                //compute taken vertices and edges
                if(idx > 0) {
                    vector<int> path = fp.paths.path(candidate, next.match[skeleton.cPrev()[idx]]);
                    for(j = 0; j < (int)path.size(); ++j)
                        next.vTaken[path[j]] = true;
                }

                //compute heuristic
                for(j = idx + 1; j < toMatch; ++j) {
                    if(skeleton.cPrev()[j] > idx)
                        continue;
                    double minP = 1e37;
                    for(k = 0; k < (int)possibilities[j].size(); ++k) {
                        minP = min(minP, computePenalty(penaltyFunctions, next, possibilities[j][k], j));
                    }
                    next.heuristic += minP;
                    if(next.heuristic > 1.)
                        break;
                }

                if(next.heuristic > 1.)
                    continue;

                todo.push(next);
            }
        }

        json js = skeleton.cPrev();
        std::cout << js.dump(4) << std::endl;
    }

    if(output.match.size() == 0)
    {
        Debugging::out() << "No Match" << endl;
    }

    for(i = 0; i < (int)penaltyFunctions.size(); ++i)
        delete penaltyFunctions[i];

    return output.match;
}

vector<Pinocchio::Vector3> splitPath(FP *fp, int joint, int curIdx, int prevIdx)
{
    int i;
    vector<int> newPath = fp->paths.path(prevIdx, curIdx);

    vector<int> uncompIdx; //stores the indices of the path in the unsimplified skeleton
    uncompIdx.push_back(fp->given.cfMap()[joint]);
    do {
        uncompIdx.push_back(fp->given.fPrev()[uncompIdx.back()]);
    } while(fp->given.fcMap()[uncompIdx.back()] == -1);
    reverse(uncompIdx.begin(), uncompIdx.end());

    vector<Pinocchio::Vector3> pathPts(uncompIdx.size(), fp->graph.verts[newPath[0]]);
    
    if(newPath.size() > 1) { //if there is a meaningful path in the extracted graph
        double dist = fp->paths.dist(newPath[0], newPath.back());
        
        vector<double> lengths(1, 0.);
        for(i = 1; i < (int)uncompIdx.size(); ++i) {
            lengths.push_back(lengths.back() + dist * fp->given.fcFraction()[uncompIdx[i]]);
        }
        
        vector<Pinocchio::Vector3> newPathPts(newPath.size());
        for(i = 0; i < (int)newPath.size(); ++i)
            newPathPts[i] = fp->graph.verts[newPath[i]];
        
        double lengthSoFar = 0;
        int curPt = 1;
        for(i = 1; i < (int)newPath.size(); ++i) {
            double len = (newPathPts[i] - newPathPts[i - 1]).length();
            if(len + lengthSoFar + 1e-6 <= lengths[curPt]) {
                lengthSoFar += len;
                continue;
            }
            double ratio = (lengths[curPt] - lengthSoFar) / len;
            pathPts[curPt] = newPathPts[i - 1] + ratio * (newPathPts[i] - newPathPts[i - 1]);
            --i; //try this segment again
            ++curPt;
            if(curPt >= (int)lengths.size())
                break;
        }
    }
    
    return pathPts;
}

vector<Pinocchio::Vector3> splitPaths(const vector<int> &discreteEmbedding, const PtGraph &graph,
                                         const Skeleton &skeleton)
{
    FP fp(graph, skeleton, vector<Sphere>());

    vector<Pinocchio::Vector3> out;

    out.push_back(graph.verts[discreteEmbedding[0]]);
    for(int i = 1; i < (int)discreteEmbedding.size(); ++i) {
        int prev = skeleton.cPrev()[i];
    
        vector<Pinocchio::Vector3> pathPts = splitPath(&fp, i, discreteEmbedding[i], discreteEmbedding[prev]);
        out.insert(out.end(), pathPts.begin() + 1, pathPts.end());
    }

    return out;
}

//-------------------------------------------------Error Features----------------------------
static const double NOMATCH = 1e10;

double smoothInterp(double val, double low, double atLow, double hi, double atHi)
{
    if(val < low)
        return atLow;
    if(val > hi)
        return atHi;
    double w = (val - low) / (hi - low);
    return w * atHi + (1. - w) * atLow;
}

static const double distPlayFactor = .7;

//distance penalty
class DistPF : public PenaltyFunction
{
public:
    DistPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        int prev = fp->given.cPrev()[idx];
        double dist = fp->paths.dist(next, cur.match[prev]);
        
        if(dist < 0) //if no path
            return NOMATCH;

        double distPlay = distPlayFactor * (fp->sph[next].radius + fp->sph[cur.match[prev]].radius); //end circle radii
        double optDist = fp->given.cLength()[idx];
        if(dist + distPlay < 0.5 * optDist) //if too short, get out
            return NOMATCH;

        double out = CUBE(smoothInterp(optDist / (dist + distPlay), 0.5, 0., 2., 3.));
        return out;
    }
};

vector<Pinocchio::Vector3> computeDirs(FP * fp, const PartialMatch &cur, int next, int idx = -1)
{
    vector<Pinocchio::Vector3> out;
    if(idx == -1)
        idx = cur.match.size();
    
    int prev = fp->given.cPrev()[idx];

    if(idx == 0 || next == cur.match[prev]) //path of zero length
        return out;

    vector<Pinocchio::Vector3> pathPts = splitPath(fp, idx, next, cur.match[prev]);
        
    out.resize(pathPts.size() - 1);

    for(int i = 0; i < (int)out.size(); ++i)
        out[i] = (pathPts[i + 1] - pathPts[i]).normalize();
    
    return out;
}

//local direction penalty
class DotPF : public PenaltyFunction
{
public:
    DotPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        double out = 0.;

        int prev = fp->given.cPrev()[idx];

        if(next == cur.match[prev]) //no penalty if same sphere
            return 0.;

        //check if the path has length 1 (like a head)
        if(fp->given.fcMap()[fp->given.fPrev()[fp->given.cfMap()[idx]]] == prev) {
            Pinocchio::Vector3 sDir = fp->given.cGraph().verts[idx] - fp->given.cGraph().verts[prev];
            Pinocchio::Vector3 dir = fp->graph.verts[next] - fp->graph.verts[cur.match[prev]];

            double dot = sDir.normalize() * dir.normalize();

            double penalty = 1. - dot;

            penalty *= smoothInterp(dot, -.5, 6., 0., 1.);

            return sDir.lengthsq() * 50. * SQR(penalty);
        }

        vector<int> uncompIdx;
        uncompIdx.push_back(fp->given.cfMap()[idx]);
        do {
            uncompIdx.push_back(fp->given.fPrev()[uncompIdx.back()]);
        } while(fp->given.fcMap()[uncompIdx.back()] == -1);
        reverse(uncompIdx.begin(), uncompIdx.end());
        
        vector<Pinocchio::Vector3> dirs = computeDirs(fp, cur, next, idx);
        if(dirs.size() == 0)
            return out;

        for(int i = 0; i < (int)dirs.size(); ++i) {
            Pinocchio::Vector3 sDir = fp->given.fGraph().verts[uncompIdx[i + 1]] - 
                fp->given.fGraph().verts[uncompIdx[i]];
            
            double dot = sDir.normalize() * dirs[i];
            
            double curPenalty = 0;

            curPenalty += SQR((1. - dot) * smoothInterp(dot, -.5, 6., 0., 1.));

            out += sDir.lengthsq() * 50. * curPenalty;
        }

        return out;
    }
};

//asymmetry penalty
class SymPF : public PenaltyFunction
{
public:
    SymPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        int prev = fp->given.cPrev()[idx];
        if(fp->given.cSym()[idx] < 0 || fp->given.cSym()[idx] >= (int)cur.match.size())
            return 0.; //doesn't apply here

        double dist = fp->paths.dist(next, cur.match[prev]);
        double distPlay = distPlayFactor * (fp->sph[next].radius + fp->sph[cur.match[prev]].radius); //end circle radii
        int v1 = fp->given.cSym()[idx];
        int v2 = fp->given.cPrev()[v1];

        double distC = dist + distPlay;
        double sDist = fp->paths.dist(cur.match[v1], cur.match[v2]);
        double sDistC = sDist + distPlayFactor * (fp->sph[cur.match[v1]].radius +
                                                  fp->sph[cur.match[v2]].radius); //add end circles
            
        double nocRatio = min(2., max(dist / (sDist + 1e-8), sDist / (dist + 1e-8)));
        double ratio = max(sDist / distC, dist / sDistC);

        return max(0., CUBE(nocRatio * 0.2 + ratio * 0.8) - 1.2);
    }
};

class GlobalDotPF : public PenaltyFunction
{
public:
    GlobalDotPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        int prev = fp->given.cPrev()[idx];
        double out = 0;
        for(int i = 0; i < (int)cur.match.size(); ++i) {
            if(i != prev && fp->given.cPrev()[i] != prev)
                    continue;
            if((fp->graph.verts[next] - fp->graph.verts[cur.match[i]]).lengthsq() < 1e-16)
                    continue;
            Pinocchio::Vector3 ourBigDir = (fp->graph.verts[next] - fp->graph.verts[cur.match[i]]);
            Pinocchio::Vector3 givenBigDir = (fp->given.cGraph().verts[idx] - fp->given.cGraph().verts[i]).normalize();
            double dot = ourBigDir.normalize() * givenBigDir;

            if(i == prev) {
                if(dot < 0.0)
                    return NOMATCH;
                
                out += 0.5 * max(0., SQR((1. - dot) * 4.) - 0.1);
            }
            else {
                if(dot < -0.5)
                    return NOMATCH;

                out += 0.5 * max(0., SQR((1. - dot) * 2.) - 0.5);
            }
        }
        return out;
    }
};

//penalizes doubled paths
class DoublePF : public PenaltyFunction
{
public:
    DoublePF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        int prev = fp->given.cPrev()[idx];

        double out = 0.;
        vector<int> newPath = fp->paths.path(next, cur.match[prev]);

        for(int i = (int)newPath.size() - 2; i >= 0; --i) { //check if tail of path is in use
            if(cur.vTaken[newPath[i]]) {
                if(fp->sph[newPath[i]].radius < 0.02) //if sphere too small to have more than one appendage
                    return NOMATCH;
                out += 0.5 / SQR(double(i + 1));
            }
        }
        return out == 0. ? 0. : out + 0.5;
    }
};

//penalizes feet off the ground
class FootPF : public PenaltyFunction
{
public:
    FootPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &, int next, int idx) const
    {
        if(fp->given.cFeet()[idx])
            return (fp->graph.verts[next][1] - fp->footBase);
        return 0;
    }
};

//penalizes duplicate nodes
class DupPF : public PenaltyFunction
{
public:
    DupPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        if(next == cur.match[fp->given.cPrev()[idx]])
            return 1.0;
        return 0.;
    }
};

//penalizes extremities that end in the middle of a path
class ExtremPF : public PenaltyFunction
{
public:
    ExtremPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        int prev = fp->given.cPrev()[idx];

        if(fp->given.cGraph().edges[idx].size() != 1 || next == cur.match[prev])
            return 0;

        Pinocchio::Vector3 ourBigDir = (fp->graph.verts[next] - fp->graph.verts[cur.match[prev]]).normalize();
        double curRad = fp->sph[next].radius;
        for(int i = 0; i < (int)fp->graph.edges[next].size(); ++i) {
            int oth = fp->graph.edges[next][i];
            if(curRad > 2. * fp->sph[oth].radius)
                continue;
            Pinocchio::Vector3 diff1 = (fp->graph.verts[oth] - fp->graph.verts[cur.match[prev]]).normalize();
            Pinocchio::Vector3 diff2 = (fp->graph.verts[oth] - fp->graph.verts[next]).normalize();
            if(diff1 * ourBigDir > 0.95 && diff2 * ourBigDir > 0.8)
                return 1.;
        }
        return 0.;
    }
};

//penalizes end vertices that are closer together in extracted graph than along their bone paths
class DisjointPF : public PenaltyFunction
{
public:
    DisjointPF(FP *inFp) : PenaltyFunction(inFp) { }
    double get(const PartialMatch &cur, int next, int idx) const
    {
        int prev = fp->given.cPrev()[idx];

        double out = 0.;

        for(int i = 0; i < (int)cur.match.size(); ++i) {
            if(i == idx || i == prev)
                continue;
            
            //compute LCA of idx and i
            int a1 = idx, a2 = i;
            while(a1 != a2) {
                if(a1 < a2)
                    a2 = fp->given.cPrev()[a2];
                else
                    a1 = fp->given.cPrev()[a1];
            }

            double sSize = fp->sph[next].radius + fp->sph[cur.match[i]].radius;
            double gDist = fp->paths.dist(next, cur.match[i]);                
            double bDist = fp->paths.dist(next, cur.match[a1]) + fp->paths.dist(cur.match[a1], cur.match[i]);

            double ratio  = (bDist + sSize) / (gDist + sSize);

            if(ratio > 2.)
                out += 1.;
        }

        return out;
    }
};

vector<PenaltyFunction *> getPenaltyFunctions(FP *fp) //user responsible for deletion of penalties
{
    vector<PenaltyFunction *> out;

    out.push_back(new DistPF(fp));
    out.push_back(new GlobalDotPF(fp));
    out.push_back(new SymPF(fp));
    out.push_back(new DoublePF(fp));
    out.push_back(new FootPF(fp));
    out.push_back(new DupPF(fp));
    out.push_back(new DotPF(fp));
    out.push_back(new ExtremPF(fp));
    out.push_back(new DisjointPF(fp));

    double weights[9] = { 0.027, 0.023, 0.007, 0.046, 0.014, 0.012, 0.072, 0.005, 0.033 };

    for(int i = 0; i < (int)out.size(); ++i)
        out[i]->weight = weights[i];

    return out;
}
