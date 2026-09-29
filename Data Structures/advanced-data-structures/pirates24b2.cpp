#include "pirates24b2.h"

oceans_t::oceans_t()
{
	// TODO: Your code goes here
}

oceans_t::~oceans_t()
{
	// TODO: Your code goes here
}



////////////////////////////////done
StatusType oceans_t::add_fleet(int fleetId)
{
	if(fleetId <= 0){
        return StatusType::INVALID_INPUT;
    }
    if(Fleets.exist(fleetId)){
        return StatusType::FAILURE;
    }
    try{
        shared_ptr<Fleet> fleet = std::make_shared<Fleet>(fleetId);
        fleet->setParent(fleet);
        Fleets.push(fleetId,fleet);
    }catch (...){
        return StatusType::ALLOCATION_ERROR;
    }
	return StatusType::SUCCESS;
}



StatusType oceans_t::add_pirate(int pirateId, int fleetId)
{
	if(pirateId <= 0 || fleetId <= 0){
        return StatusType::INVALID_INPUT;
    }
    if(Pirates.exist(pirateId) || !Fleets.exist(fleetId)){
        return StatusType::FAILURE;
    }
    shared_ptr<Fleet> fleet = Fleets.find(fleetId);
    if(fleet->getAvilability() == false){
        return StatusType::FAILURE;
    }
    try {
        shared_ptr<Pirate> pirate = std::make_shared<Pirate>(pirateId,fleet->getId(),fleet->getSize());
        Pirates.push(pirateId,pirate);
        fleet->setSize(fleet->getSize()+1);
        //////////////////////////////////
    }catch (...){
        return StatusType::ALLOCATION_ERROR;
    }
	return StatusType::SUCCESS;
}

///////////////////////////////done
StatusType oceans_t::pay_pirate(int pirateId, int salary)
{
	if(pirateId <= 0 || salary <= 0){
        return StatusType::INVALID_INPUT;
    }
    if (!Pirates.exist(pirateId)){
        return StatusType::FAILURE;
    }
    shared_ptr<Pirate> pirate = Pirates.find(pirateId);
    pirate->setSalary(pirate->getSalary() + salary);
	return StatusType::SUCCESS;
}


output_t<int> oceans_t::num_ships_for_fleet(int fleetId)
{
    if(fleetId <=0){
        return StatusType::INVALID_INPUT;
    }
    if(!Fleets.exist(fleetId)){
        return StatusType::FAILURE;
    }
    std::shared_ptr<Fleet> fleet = Fleets.find(fleetId);
    if(fleet->getAvilability() == false){
        return StatusType::FAILURE;
    }
    shared_ptr<Fleet> root = unionFind.find(fleet);
    return root->getNumOfShips();

}


//////////////////////done
output_t<int> oceans_t::get_pirate_money(int pirateId)
{
    if (pirateId <= 0){
        return StatusType::INVALID_INPUT;
    }
    if(!Pirates.exist(pirateId)){
        return StatusType::FAILURE;
    }
    return Pirates.find(pirateId)->getSalary();
}

/////////////////////////////done
StatusType oceans_t::unite_fleets(int fleetId1, int fleetId2)
{
	if(fleetId1 <= 0 || fleetId2 <= 0 || fleetId1 == fleetId2){
        return StatusType::INVALID_INPUT;
    }
    if(!Fleets.exist(fleetId1) || !Fleets.exist(fleetId2)){
        return StatusType::FAILURE;
    }
    shared_ptr<Fleet> fleet1 = Fleets.find(fleetId1);
    shared_ptr<Fleet> fleet2 = Fleets.find(fleetId2);
    if(!fleet1->getAvilability() || !fleet2->getAvilability()){
        return StatusType::FAILURE;
    }
    if(fleet1->getSize() == 0 || fleet2->getSize() == 0){
        return StatusType::FAILURE;
    }
    unionFind.unite(fleet1,fleet2);
    return StatusType::SUCCESS;
}



///////////done
StatusType oceans_t::pirate_argument(int pirateId1, int pirateId2)
{
    if(pirateId1 <= 0 || pirateId2 <= 0 || pirateId1 == pirateId2){
        return StatusType::INVALID_INPUT;        
    }
    if(!Pirates.exist(pirateId1) || !Pirates.exist(pirateId2)){
        return StatusType::FAILURE;
    }
    shared_ptr<Pirate> pirate1 = Pirates.find(pirateId1);
    shared_ptr<Pirate> pirate2 = Pirates.find(pirateId2);
    shared_ptr<Fleet> fleet1=Fleets.find(pirate1->getFlleetId());
    shared_ptr<Fleet> fleet2=Fleets.find(pirate2->getFlleetId());
    
    if(unionFind.find(fleet1)->getId()!= unionFind.find(fleet2)->getId()){
        return StatusType::FAILURE;
    }
   
    int rank1 = pirate1->getRank()+ fleet1->getAddTo()+fleet1->getParent()->getAddTo();
    int rank2 = pirate2->getRank()+ fleet2->getAddTo()+fleet2->getParent()->getAddTo();    
    if(rank1 < rank2){
        pirate1->setSalary(pirate1->getSalary() + (rank2 - rank1));
        pirate2->setSalary(pirate2->getSalary() - (rank2 - rank1));
    }
    else {
        pirate2->setSalary(pirate2->getSalary() + (rank1 - rank2));
        pirate1->setSalary(pirate1->getSalary() - (rank1 - rank2));
    }
    return StatusType::SUCCESS;
}
