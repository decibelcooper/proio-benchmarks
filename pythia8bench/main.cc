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

    auto writer = new proio::Writer("particles.proio");
    auto writerUncomp = new proio::Writer("particles_uncomp.proio");
    writerUncomp->SetCompression(proio::UNCOMPRESSED);
    auto event = new proio::Event();
    auto partDesc = DescriptorPool::generated_pool()->FindMessageTypeByName("proio.model.example.Particle");
    auto varintWriter = new proio::Writer("varint_particles.proio");
    auto varintWriterUncomp = new proio::Writer("varint_particles_uncomp.proio");
    varintWriterUncomp->SetCompression(proio::UNCOMPRESSED);
    auto varintEvent = new proio::Event();
    auto varintPartDesc =
        DescriptorPool::generated_pool()->FindMessageTypeByName("proio.model.example.VarintParticle");
    auto packedWriter = new proio::Writer("packed_particles.proio");
    auto packedWriterUncomp = new proio::Writer("packed_particles_uncomp.proio");
    packedWriterUncomp->SetCompression(proio::UNCOMPRESSED);
    auto packedEvent = new proio::Event();
    auto packedParts = new model::PackedParticles();
    auto packedPartsDesc = packedParts->GetDescriptor();
    auto varintPackedWriter = new proio::Writer("varint_packed_particles.proio");
    auto varintPackedWriterUncomp = new proio::Writer("varint_packed_particles_uncomp.proio");
    varintPackedWriterUncomp->SetCompression(proio::UNCOMPRESSED);
    auto varintPackedEvent = new proio::Event();
    auto varintPackedParts = new model::VarintPackedParticles();
    auto varintPackedPartsDesc = varintPackedParts->GetDescriptor();
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
            varintVertex->set_t(pythiaPart.tProd() / 3e-1);
            auto varintP = varintPart->mutable_p();
            varintP->set_x(pythiaPart.px() * 1e3);
            varintP->set_y(pythiaPart.py() * 1e3);
            varintP->set_z(pythiaPart.pz() * 1e3);
            varintPart->set_mass(pythiaPart.m() * 1e3);
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
            packedParts->add_t(pythiaPart.tProd() / 3e2);
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
            varintPackedParts->add_t(pythiaPart.tProd() / 3e-1);
            varintPackedParts->add_px(pythiaPart.px() * 1e3);
            varintPackedParts->add_py(pythiaPart.py() * 1e3);
            varintPackedParts->add_pz(pythiaPart.pz() * 1e3);
            varintPackedParts->add_mass(pythiaPart.m() * 1e3);
            varintPackedParts->add_charge(3 * pythiaPart.charge());
        }  // end loop over particles

        writer->Push(event);
        writerUncomp->Push(event);
        event->Clear();
        varintWriter->Push(varintEvent);
        varintWriterUncomp->Push(varintEvent);
        varintEvent->Clear();
        packedEvent->AddEntry(packedParts, "Particle");
        packedWriter->Push(packedEvent);
        packedWriterUncomp->Push(packedEvent);
        packedEvent->Clear();
        packedParts = (model::PackedParticles *)packedEvent->Free(packedPartsDesc);
        varintPackedEvent->AddEntry(varintPackedParts, "Particle");
        varintPackedWriter->Push(varintPackedEvent);
        varintPackedWriterUncomp->Push(varintPackedEvent);
        varintPackedEvent->Clear();
        varintPackedParts = (model::VarintPackedParticles *)varintPackedEvent->Free(varintPackedPartsDesc);
    }  // end of loop over events

    delete event;
    delete varintEvent;
    delete packedEvent;
    delete varintPackedEvent;
    delete writer;
    delete writerUncomp;
    delete varintWriter;
    delete varintWriterUncomp;
    delete packedWriter;
    delete packedWriterUncomp;
    delete varintPackedWriter;
    delete varintPackedWriterUncomp;

    exit(EXIT_SUCCESS);
}
