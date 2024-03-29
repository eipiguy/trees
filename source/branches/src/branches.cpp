#include <iostream>
#include <math.h>

#include "branches.hpp"

using std::cout;
using std::endl;

///////////////////////////////////////////////////////////////////////////////

Branch :: Branch()
    :   profile(CurveProfile(Point(0,0,0), Direction(0,0,1))) {
    Point endPt = profile.getEnd();
    for(size_t i=0; i<LEAVES_PER_BRANCH; ++i) {
        leaves[i] = Leaf();
    }
}

//=============================================================================

void Branch :: resetChildBasePoints() {
    Point currentEnd = profile.getEnd();

    for( auto &leaf : leaves ) {
        leaf.base = currentEnd;
    }

    for( auto &child : children ) {
        child->profile.base = currentEnd;
        child->resetChildBasePoints();
    }
}

void Branch :: addChild() {
    auto newChild = std::make_unique<Branch>();
    int numAttempts = 4;
    while( measureMinRadAngle(*newChild) < DEFLECTION_RAD && --numAttempts > 0 ) {
        adjustNewChild(newChild);
        cout << "Min radian angle to children = ";
        cout << measureMinRadAngle(*newChild) << endl;
    }

    if( numAttempts > 0 ) {
        children.emplace_back(std::move(newChild));
        resetChildBasePoints();
    }
}

void Branch :: adjustNewChild( std::unique_ptr<Branch> &newChild ) {
    double radAngleToChild = 0;
    Direction averageResult = { 0, 0, 0 };
    Direction thisResult = { 0, 0, 0 };
    bool collision = false;
    for( auto &child : children ) {
        radAngleToChild = abs( child->measureRadAngle(*newChild) );
        if( radAngleToChild < DEFLECTION_RAD ) {
            collision = true;
            cout << "Radian angle to newChild = " << radAngleToChild << " is less than allowed " << DEFLECTION_RAD << endl;
            cout << "Rotating away" << endl;

            Direction crossNew = cross( ( 1 / child->profile.length.length() ) * child->profile.length, ( 1 / newChild->profile.length.length() ) * newChild->profile.length );

            if( crossNew.length() < MIN_LENGTH ) {
                crossNew = { 1, 0, 0 };
            }

            thisResult = newChild->profile.length;
            thisResult.rotate( DEFLECTION_RAD - radAngleToChild, crossNew );
            averageResult += thisResult;
            thisResult = { 0, 0, 0 };
        }
    }

    if(collision) {
        averageResult *= 1 / averageResult.length();
        newChild->profile.length = averageResult;
    }
}

void Branch :: addChild( std::unique_ptr<Branch> &child ) {
    children.emplace_back(std::move(child));
    resetChildBasePoints();
}

Direction Branch :: getTipToTail(){
    return profile.length;
}

double Branch :: measureRadAngle( Branch &comparison ) {
    return acos( ( getTipToTail() * comparison.getTipToTail() ) / ( profile.pathLength() * comparison.profile.pathLength() ) );
}

double Branch :: measureRadAngle( size_t i, size_t j ) {
    return acos( ( children[0]->getTipToTail() * children[1]->getTipToTail() ) / ( children[0]->profile.pathLength() * children[1]->profile.pathLength() ) );
}

double Branch :: measureMinRadAngle( Branch &comparison ) {
    double radAngleToChild = 0;
    double minRadAngleToChild = 2;
    for( auto &child : children ) {
        radAngleToChild = abs( child->measureRadAngle(comparison) );
        if( radAngleToChild < minRadAngleToChild ) {
            minRadAngleToChild = radAngleToChild;
        }
    }
    return minRadAngleToChild;
}

double Branch :: minDistance( Branch &compare ) {
    return profile.minDistance(compare.profile);
}

void Branch :: rotate( const double angle, Direction &normalAxis ) {
    profile.rotate( angle, normalAxis );
}

//=============================================================================

size_t Branch :: getNumChildren() { return children.size(); }

size_t Branch :: getNumLeaves() { return leaves.size(); }

double Branch :: getDiameter() { return diameter; }

//=============================================================================

void Branch :: print() {
    cout << endl << "Curve Profile:" << endl;
    profile.print();

    cout << endl << " Leaves (" << getNumLeaves() << "):" << endl;
    for( auto &leaf : leaves ) {
        leaf.print();
    }

    cout << endl << " Children (" << getNumChildren() << "):" << endl;
    for( auto &child : children ) {
        child->print();
    }
}

///////////////////////////////////////////////////////////////////////////////

Branch :: CurveProfile :: CurveProfile( const Point &baseIn, const Direction &lengthIn )
    : base(baseIn), length(lengthIn) {}

double Branch :: CurveProfile :: pathLength() {
    return sqrt(length*length);
}

Point Branch :: CurveProfile :: getPoint( const double &param ) {
    return base + (param * length);
}

Point Branch :: CurveProfile :: getEnd() {
    return base + length;
}

double Branch :: CurveProfile :: measureRadAngle( const CurveProfile &comparison ) {
    return length*comparison.length;
}

double Branch :: CurveProfile :: minDistance( CurveProfile &compare ) {
    Direction normal = cross( length, compare.length );
    normal.normalize();
    return abs( ( base - compare.base ) * normal );
}

void Branch :: CurveProfile :: rotate( const double angle, Direction &normalAxis ) {
    length.rotate( angle, normalAxis );
}

void Branch :: CurveProfile :: print() {
    cout << "base point: ";
    base.print();

    cout << "length vector: ";
    length.print();

    cout << "path length = " << pathLength() << endl;

    cout << "end point: ";
    getEnd().print();
}

///////////////////////////////////////////////////////////////////////////////

Leaf :: Leaf( const Point &baseIn, const Direction &meridianIn, const double occlusionIn ) 
:   base(baseIn),
    meridian(meridianIn),
    occlusion(occlusionIn) {}

Point Leaf :: getEnd() { return base + meridian; }

void Leaf :: print() {
    cout << "occlusion = " << occlusion << endl;

    cout << "base point: ";
    
    base.print();

    cout << "length vector: ";
    meridian.print();

    cout << "total length = " << meridian.length() << endl;

    cout << "end point: ";
    getEnd().print();
}