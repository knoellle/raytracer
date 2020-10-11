#include "scene.hpp"
#include "entity.hpp"

static int ID_counter = 0;
struct KDN
{
    int ID;
    KDN(){
        ID = ID_counter++;
    }
    AABB boundingBox;
    int axis;
    int depth;
    bool f;
    std::shared_ptr<KDN> left;
    std::shared_ptr<KDN> right;
    std::vector<std::shared_ptr<Entity>> entities;
    virtual bool hit(const Ray& r, const double t_min, const double t_max, HitData &data) const;
};

class KDTreeScene: public Scene
{
    public:
        KDTreeScene();

        virtual void update() override;
        virtual bool hit(const Ray& r, const double t_min, const double t_max, HitData &data) const override;

    private:
        std::shared_ptr<KDN> construct(std::vector<std::shared_ptr<Entity>> entities, const int depth);
        std::shared_ptr<KDN> rootNode;
};

bool KDN::hit(const Ray& r, const double t_min, const double t_max, HitData &data) const
{
    if (!boundingBox.intersect(r))
    {
        return false;
    }
    if (depth == data.debugStep)
    {
        data.debugColor = f ? Vec3(1,0,0) : Vec3(0,1,0);
        // data.debug = true;
        data.debugCounter++;
    }

    std::shared_ptr<KDN> first;
    std::shared_ptr<KDN> second;
    if (r.direction()[axis] > 0)
    {
        first = left;
        second = right;
    }
    else
    {
        first = right;
        second = left;
    }
    
    double t_far = t_max;
    bool hit_first = false;
    if (first)
    {
        hit_first = first->hit(r, t_min, t_far, data);
        if (hit_first)
        {
            t_far = data.t;
        }
    }
    bool hit_second = false;
    if (second)
    {
        hit_second = second->hit(r, t_min, t_far, data);
        if (hit_second)
        {
            t_far = data.t;
        }
    }
    if (hit_first || hit_second)
    {
        return true;
    }

    HitData temp_data;
    double closest_hit = t_far;
    for (const auto e : entities)
    {
        if (e->hit(r, t_min, closest_hit, data))
        {
            closest_hit = data.t;
        }
    }
    return closest_hit < t_max;
}

KDTreeScene::KDTreeScene()
{
}

void KDTreeScene::update()
{
    Scene::update();
    rootNode = construct(entities, 0);
}

bool KDTreeScene::hit(const Ray& r, const double t_min, const double t_max, HitData &data) const
{
    // A tree exists, use it to find hit points
    if (rootNode)
    {
        return rootNode->hit(r, t_min, t_max, data);
    }

    // There is no tree, naively check all entities
    double closest_hit = t_max;
    for (const auto &e : entities)
    {
        if (e->hit(r, t_min, closest_hit, data))
        {
            closest_hit = data.t;
        }
    }
    return closest_hit < t_max;
}

std::shared_ptr<KDN> KDTreeScene::construct(const std::vector<std::shared_ptr<Entity>> entities, const int depth)
{
    auto node = std::make_shared<KDN>();
    node->left = 0;
    node->right = 0;
    node->depth = depth;

    if (entities.size() == 0)
    {
        std::cout << "empty leaf " << depth << std::endl;
        return node;
    }

    node->boundingBox = entities[0]->boundingBox;

    Vec3 averagePosition(0.0f);
    for (auto& entity : entities)
    {
        averagePosition += entity->transform;
        node->boundingBox.expand(entity->boundingBox);
    }
    averagePosition /= entities.size();

    if (entities.size() <= 4 || depth > 8)
    {
        std::cout << "leaf with " << entities.size() << " objects " << depth << std::endl;
        for (auto &e : entities)
        {
            node->entities.emplace_back(e);
        }
        return node;
    }


    // Find largest bounding box dimension
    const Vec3 boundingBoxSize = node->boundingBox.high - node->boundingBox.low;
    int splitAxis;
    if (boundingBoxSize[0] >= boundingBoxSize[1] && boundingBoxSize[0] >= boundingBoxSize[2])
    {
        splitAxis = 0;
    }
    else
    {
        splitAxis = boundingBoxSize[1] >= boundingBoxSize[2] ? 1 : 2;
    }
    node->axis = splitAxis;
    
    std::cout << "Axis: " << splitAxis << std::endl;

    std::vector<std::shared_ptr<Entity>> leftEntities;
    std::vector<std::shared_ptr<Entity>> rightEntities;
    for (auto& entity : entities)
    {
        if (entity->transform[splitAxis] < averagePosition[splitAxis])
        {
            leftEntities.emplace_back(entity);
        }
        else
        {
            rightEntities.emplace_back(entity);
        }
    }

    if (leftEntities.size() > 0)
    {
        std::cout << "constructing left node" << std::endl;
        node->left = construct(leftEntities, depth + 1);
        node->boundingBox.expand(node->left->boundingBox);
        node->left->f = true;
    }
    if (rightEntities.size() > 0)
    {
        std::cout << "constructing right node" << std::endl;
        node->right = construct(rightEntities, depth + 1);
        node->boundingBox.expand(node->right->boundingBox);
        node->right->f = false;
    }

    return node;
}
