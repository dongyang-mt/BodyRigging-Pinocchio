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

ostream *Debugging::outStream = new ofstream();

#ifdef _WIN32
#include "windows.h"

long getSysT()
{
    SYSTEMTIME systime;
    GetSystemTime(&systime);

    return systime.wMilliseconds + 1000 * (systime.wSecond + 60 * (systime.wMinute + 60 * (systime.wHour + 24 * systime.wDay)));
}
#else
#include <sys/time.h>

long getT()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

int getSysMsecs()
{
    static unsigned long startTime = getSysT();
    return getSysT() - startTime;
}

PinocchioOutput autorig(const Skeleton &given, const Mesh &m)
{
    int i;
    PinocchioOutput out;

    Mesh newMesh = prepareMesh(m);

    if(newMesh.vertices.size() == 0)
        return out;
    int startTime = getSysMsecs();
    TreeType *distanceField = constructDistanceField(newMesh);
    Debugging::out() << "constructDistanceField cost time: " << getSysMsecs() - startTime << std::endl;

    //discretization
    startTime = getSysMsecs();
    vector<Sphere> medialSurface = sampleMedialSurface(distanceField);
    Debugging::out() << "sampleMedialSurface cost time: " << getSysMsecs() - startTime << std::endl;

    startTime = getSysMsecs();
    vector<Sphere> spheres = packSpheres(medialSurface);
    Debugging::out() << "packSpheres cost time: " << getSysMsecs() - startTime << std::endl;

    startTime = getSysMsecs();
    PtGraph graph = connectSamples(distanceField, spheres);
    Debugging::out() << "connectSamples cost time: " << getSysMsecs() - startTime << std::endl;

    //discrete embedding
    startTime = getSysMsecs();
    vector<vector<int> > possibilities = computePossibilities(graph, spheres, given);
    Debugging::out() << "computePossibilities cost time: " << getSysMsecs() - startTime << std::endl;

    //constraints can be set by respecifying possibilities for skeleton joints:
    //to constrain joint i to sphere j, use: possiblities[i] = vector<int>(1, j);

    startTime = getSysMsecs();
    vector<int> embeddingIndices = discreteEmbed(graph, spheres, given, possibilities);
    Debugging::out() << "discreteEmbed cost time: " << getSysMsecs() - startTime << std::endl;

    if(embeddingIndices.size() == 0) { //failure
        delete distanceField;
        return out;
    }

    startTime = getSysMsecs();
    vector<Pinocchio::Vector3> discreteEmbedding = splitPaths(embeddingIndices, graph, given);
    Debugging::out() << "splitPaths cost time: " << getSysMsecs() - startTime << std::endl;

    //continuous refinement
    startTime = getSysMsecs();
    vector<Pinocchio::Vector3> medialCenters(medialSurface.size());
    for(i = 0; i < (int)medialSurface.size(); ++i)
        medialCenters[i] = medialSurface[i].center;
    Debugging::out() << "medialCenters cost time: " << getSysMsecs() - startTime << std::endl;

    startTime = getSysMsecs();
    out.embedding = refineEmbedding(distanceField, medialCenters, discreteEmbedding, given);
    Debugging::out() << "refineEmbedding cost time: " << getSysMsecs() - startTime << std::endl;

    //attachment
    VisTester<TreeType> *tester = new VisTester<TreeType>(distanceField);
    out.attachment = new Attachment(newMesh, given, out.embedding, tester);

    //cleanup
    delete tester;
    delete distanceField;

    return out;
}

