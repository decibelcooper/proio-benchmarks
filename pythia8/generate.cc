// Example code that runs Pythia8 and creates ProIO and ROOT files
// S.Chekanov (ANL) and D. Blyth (ANL)

#include <getopt.h>
#include <stdio.h>
#include <time.h>
#include <limits>
#include <map>

#include <Pythia8/Pythia.h>

#include <google/protobuf/descriptor.h>
#include <proio/event.h>
#include <proio/model/example/example.pb.h>
#include <proio/writer.h>

#include <Compression.h>
#include <TFile.h>
#include <TTree.h>

using namespace google::protobuf;
namespace model = proio::model::example;

void printUsage(char **argv) { std::cerr << "Usage: " << argv[0] << " cardPath [nEvents]" << std::endl; }

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            default:
                printUsage(argv);
                exit(EXIT_FAILURE);
        }
    }

    char *card = NULL;
    if (optind < argc)
        card = argv[optind];
    else {
        printUsage(argv);
        exit(EXIT_FAILURE);
    }

    int nEvents = 0;
    if (optind + 1 < argc) nEvents = atoi(argv[optind + 1]);

    Pythia8::Pythia pythia;
    pythia.readFile(card);
    pythia.init();

    auto writer = new proio::Writer("particles_uncomp.proio");
    writer->SetCompression(proio::UNCOMPRESSED);
    auto event = new proio::Event();
    auto partDesc = DescriptorPool::generated_pool()->FindMessageTypeByName("proio.model.example.Particle");
    auto varintWriter = new proio::Writer("varint_particles_uncomp.proio");
    varintWriter->SetCompression(proio::UNCOMPRESSED);
    auto varintEvent = new proio::Event();
    auto varintPartDesc =
        DescriptorPool::generated_pool()->FindMessageTypeByName("proio.model.example.VarintParticle");
    auto packedWriter = new proio::Writer("packed_particles_uncomp.proio");
    packedWriter->SetCompression(proio::UNCOMPRESSED);
    auto packedEvent = new proio::Event();
    auto packedParts = new model::PackedParticles();
    auto packedPartsDesc = packedParts->GetDescriptor();
    auto varintPackedWriter = new proio::Writer("varint_packed_particles_uncomp.proio");
    varintPackedWriter->SetCompression(proio::UNCOMPRESSED);
    auto varintPackedEvent = new proio::Event();
    auto varintPackedParts = new model::VarintPackedParticles();
    auto varintPackedPartsDesc = varintPackedParts->GetDescriptor();

    TFile rootFile("packed_particles_uncomp.root", "recreate");
    rootFile.SetCompressionLevel(0);
    auto partTree = new TTree("particles", "Particles");
    partTree->SetAutoFlush(1000);
    ROOT::TIOFeatures features;
    features.Set(ROOT::Experimental::EIOFeatures::kGenerateOffsetMap);
    partTree->SetIOFeatures(features);
    uint32_t n;
    auto nBranch = partTree->Branch("n", &n, "n/i");
    auto parent1Branch = partTree->Branch("parent1", 0, "parent1[n]/i");
    auto parent2Branch = partTree->Branch("parent2", 0, "parent2[n]/i");
    auto child1Branch = partTree->Branch("child1", 0, "child1[n]/i");
    auto child2Branch = partTree->Branch("child2", 0, "child2[n]/i");
    auto pdgBranch = partTree->Branch("pdg", 0, "pdg[n]/I");
    auto xBranch = partTree->Branch("x", 0, "x[n]/F");
    auto yBranch = partTree->Branch("y", 0, "y[n]/F");
    auto zBranch = partTree->Branch("z", 0, "z[n]/F");
    auto tBranch = partTree->Branch("t", 0, "y[n]/F");
    auto pxBranch = partTree->Branch("px", 0, "px[n]/F");
    auto pyBranch = partTree->Branch("py", 0, "py[n]/F");
    auto pzBranch = partTree->Branch("pz", 0, "pz[n]/F");
    auto massBranch = partTree->Branch("mass", 0, "mass[n]/F");
    auto chargeBranch = partTree->Branch("charge", 0, "charge[n]/i");

    TFile rootFileInt("int_packed_particles_uncomp.root", "recreate");
    rootFileInt.SetCompressionLevel(0);
    auto partTreeInt = new TTree("particles", "Particles");
    partTreeInt->SetAutoFlush(1000);
    partTreeInt->SetIOFeatures(features);
    auto nBranchInt = partTreeInt->Branch("n", &n, "n/i");
    auto parent1BranchInt = partTreeInt->Branch("parent1", 0, "parent1[n]/i");
    auto parent2BranchInt = partTreeInt->Branch("parent2", 0, "parent2[n]/i");
    auto child1BranchInt = partTreeInt->Branch("child1", 0, "child1[n]/i");
    auto child2BranchInt = partTreeInt->Branch("child2", 0, "child2[n]/i");
    auto pdgBranchInt = partTreeInt->Branch("pdg", 0, "pdg[n]/I");
    auto xBranchInt = partTreeInt->Branch("x", 0, "x[n]/I");
    auto yBranchInt = partTreeInt->Branch("y", 0, "y[n]/I");
    auto zBranchInt = partTreeInt->Branch("z", 0, "z[n]/I");
    auto tBranchInt = partTreeInt->Branch("t", 0, "t[n]/I");
    auto pxBranchInt = partTreeInt->Branch("px", 0, "px[n]/I");
    auto pyBranchInt = partTreeInt->Branch("py", 0, "py[n]/I");
    auto pzBranchInt = partTreeInt->Branch("pz", 0, "pz[n]/I");
    auto massBranchInt = partTreeInt->Branch("mass", 0, "mass[n]/i");
    auto chargeBranchInt = partTreeInt->Branch("charge", 0, "charge[n]/i");

    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) {
            i--;
            continue;
        }

        for (int j = 0; j < pythia.event.size(); j++) {
            Pythia8::Particle &pythiaPart = pythia.event[j];

            auto part = (model::Particle *)event->Free(partDesc);
            if (!part) part = new model::Particle();
            if (pythiaPart.mother1() != 0) part->add_parent(pythiaPart.mother1());
            if (pythiaPart.mother2() != 0) part->add_parent(pythiaPart.mother2());
            if (pythiaPart.daughter1() != 0) part->add_child(pythiaPart.daughter1());
            if (pythiaPart.daughter2() != 0) part->add_child(pythiaPart.daughter2());
            part->set_pdg(pythiaPart.id());
            auto vertex = part->mutable_vertex();
            vertex->set_x(pythiaPart.xProd());
            vertex->set_y(pythiaPart.yProd());
            vertex->set_z(pythiaPart.zProd());
            vertex->set_t(pythiaPart.tProd() / 3e2);
            auto p = part->mutable_p();
            p->set_x(pythiaPart.px());
            p->set_y(pythiaPart.py());
            p->set_z(pythiaPart.pz());
            part->set_mass(pythiaPart.m());
            part->set_charge(3 * pythiaPart.charge());
            event->AddEntry(part, "Particle");

            auto varintPart = (model::VarintParticle *)varintEvent->Free(varintPartDesc);
            if (!varintPart) varintPart = new model::VarintParticle();
            if (pythiaPart.mother1() != 0) varintPart->add_parent(pythiaPart.mother1());
            if (pythiaPart.mother2() != 0) varintPart->add_parent(pythiaPart.mother2());
            if (pythiaPart.daughter1() != 0) varintPart->add_child(pythiaPart.daughter1());
            if (pythiaPart.daughter2() != 0) varintPart->add_child(pythiaPart.daughter2());
            varintPart->set_pdg(pythiaPart.id());
            auto varintVertex = varintPart->mutable_vertex();
            varintVertex->set_x(pythiaPart.xProd() * 1e3);
            varintVertex->set_y(pythiaPart.yProd() * 1e3);
            varintVertex->set_z(pythiaPart.zProd() * 1e3);
            varintVertex->set_t(pythiaPart.tProd() * 1e3);
            auto varintP = varintPart->mutable_p();
            varintP->set_x(pythiaPart.px() * 1e5);
            varintP->set_y(pythiaPart.py() * 1e5);
            varintP->set_z(pythiaPart.pz() * 1e5);
            varintPart->set_mass(pythiaPart.m() * 1e5);
            varintPart->set_charge(3 * pythiaPart.charge());
            varintEvent->AddEntry(varintPart, "Particle");

            packedParts->add_parent1(pythiaPart.mother1());
            packedParts->add_parent2(pythiaPart.mother2());
            packedParts->add_child1(pythiaPart.daughter1());
            packedParts->add_child2(pythiaPart.daughter2());
            packedParts->add_pdg(pythiaPart.id());
            packedParts->add_x(pythiaPart.xProd());
            packedParts->add_y(pythiaPart.yProd());
            packedParts->add_z(pythiaPart.zProd());
            packedParts->add_t(pythiaPart.tProd());
            packedParts->add_px(pythiaPart.px());
            packedParts->add_py(pythiaPart.py());
            packedParts->add_pz(pythiaPart.pz());
            packedParts->add_mass(pythiaPart.m());
            packedParts->add_charge(3 * pythiaPart.charge());

            varintPackedParts->add_parent1(pythiaPart.mother1());
            varintPackedParts->add_parent2(pythiaPart.mother2());
            varintPackedParts->add_child1(pythiaPart.daughter1());
            varintPackedParts->add_child2(pythiaPart.daughter2());
            varintPackedParts->add_pdg(pythiaPart.id());
            varintPackedParts->add_x(pythiaPart.xProd() * 1e3);
            varintPackedParts->add_y(pythiaPart.yProd() * 1e3);
            varintPackedParts->add_z(pythiaPart.zProd() * 1e3);
            varintPackedParts->add_t(pythiaPart.tProd() * 1e3);
            varintPackedParts->add_px(pythiaPart.px() * 1e5);
            varintPackedParts->add_py(pythiaPart.py() * 1e5);
            varintPackedParts->add_pz(pythiaPart.pz() * 1e5);
            varintPackedParts->add_mass(pythiaPart.m() * 1e5);
            varintPackedParts->add_charge(3 * pythiaPart.charge());
        }  // end loop over particles

        writer->Push(event);
        event->Clear();
        varintWriter->Push(varintEvent);
        varintEvent->Clear();
        packedEvent->AddEntry(packedParts, "Particle");
        packedWriter->Push(packedEvent);
        packedEvent->Clear();
        packedParts = (model::PackedParticles *)packedEvent->Free(packedPartsDesc);
        varintPackedEvent->AddEntry(varintPackedParts, "Particle");
        varintPackedWriter->Push(varintPackedEvent);
        varintPackedEvent->Clear();
        varintPackedParts = (model::VarintPackedParticles *)varintPackedEvent->Free(varintPackedPartsDesc);

        n = pythia.event.size();

        parent1Branch->SetAddress(packedParts->mutable_parent1()->mutable_data());
        parent2Branch->SetAddress(packedParts->mutable_parent2()->mutable_data());
        child1Branch->SetAddress(packedParts->mutable_child1()->mutable_data());
        child2Branch->SetAddress(packedParts->mutable_child2()->mutable_data());
        pdgBranch->SetAddress(packedParts->mutable_pdg()->mutable_data());
        xBranch->SetAddress(packedParts->mutable_x()->mutable_data());
        yBranch->SetAddress(packedParts->mutable_y()->mutable_data());
        zBranch->SetAddress(packedParts->mutable_z()->mutable_data());
        tBranch->SetAddress(packedParts->mutable_y()->mutable_data());
        pxBranch->SetAddress(packedParts->mutable_px()->mutable_data());
        pyBranch->SetAddress(packedParts->mutable_py()->mutable_data());
        pzBranch->SetAddress(packedParts->mutable_pz()->mutable_data());
        massBranch->SetAddress(packedParts->mutable_mass()->mutable_data());
        chargeBranch->SetAddress(packedParts->mutable_charge()->mutable_data());
        partTree->Fill();

        parent1BranchInt->SetAddress(varintPackedParts->mutable_parent1()->mutable_data());
        parent2BranchInt->SetAddress(varintPackedParts->mutable_parent2()->mutable_data());
        child1BranchInt->SetAddress(varintPackedParts->mutable_child1()->mutable_data());
        child2BranchInt->SetAddress(varintPackedParts->mutable_child2()->mutable_data());
        pdgBranchInt->SetAddress(varintPackedParts->mutable_pdg()->mutable_data());
        xBranchInt->SetAddress(varintPackedParts->mutable_x()->mutable_data());
        yBranchInt->SetAddress(varintPackedParts->mutable_y()->mutable_data());
        zBranchInt->SetAddress(varintPackedParts->mutable_z()->mutable_data());
        tBranchInt->SetAddress(varintPackedParts->mutable_y()->mutable_data());
        pxBranchInt->SetAddress(varintPackedParts->mutable_px()->mutable_data());
        pyBranchInt->SetAddress(varintPackedParts->mutable_py()->mutable_data());
        pzBranchInt->SetAddress(varintPackedParts->mutable_pz()->mutable_data());
        massBranchInt->SetAddress(varintPackedParts->mutable_mass()->mutable_data());
        chargeBranchInt->SetAddress(varintPackedParts->mutable_charge()->mutable_data());
        partTreeInt->Fill();
    }  // end of loop over events

    partTree->Write();
    partTreeInt->Write();
    rootFile.Close();
    rootFileInt.Close();

    delete event;
    delete varintEvent;
    delete packedEvent;
    delete varintPackedEvent;
    delete writer;
    delete varintWriter;
    delete packedWriter;
    delete varintPackedWriter;

    exit(EXIT_SUCCESS);
}
