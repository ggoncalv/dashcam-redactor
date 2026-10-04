# Privacy model

Dashcam footage can identify people through faces, plates, locations, voices,
and context. The tool is local-first: ordinary local videos require no cloud
upload or credentials. Raw footage, intermediate frames, and outputs are
intended to remain on the machine. Users remain responsible for local permissions,
backups, storage, and external synchronization of their chosen directories.

M0 re-encodes video and provides **no visual privacy protection**. Sensitive
details remain visible. Omitting audio and metadata does not anonymize video.
Temporary files are sensitive too. Scoped cleanup attempts to remove owned
staging files on normal completion and failure, and reports cleanup errors.
Cleanup failure or forced termination can leave files; deletion does not
guarantee secure erasure.

Future detection/redaction must bias toward privacy: a false negative exposes
an identifier, while a false positive unnecessarily obscures footage. Missed
identifiers are more costly here. Evaluation should include glare, blur, distance,
occlusion, and lighting, rather than relying only on average benchmark accuracy.

Future **static privacy regions** would let users obscure fixed areas containing
burned-in date/time, GPS coordinates, device IDs, and other identifying overlays.
These details are part of the image pixels and remain visible when container
metadata is omitted. Region coverage would require human review; this feature is
not implemented in M0.

No automated system can guarantee anonymization. Human review remains necessary
before publication, including missed identifiers and contextual or audible
information. The project must never claim universally safe or guaranteed
anonymous output.

Private test media must never be committed, uploaded, or attached to issues,
pull requests, or security reports. Keep it outside the repository. Outputs must
also stay untracked. `.gitignore` is a guardrail, not access control: force-add
can bypass it. Tests generate synthetic media and must not depend on a private
directory. Share safe reproductions and only necessary non-identifying facts;
remove private filenames, locations, and unrelated metadata.

Future model selection must consider technical quality, false negatives, training
data provenance, redistribution conditions, and model/data licenses. Apache-2.0
for this repository does not automatically license weights or dependencies.
Review and document licenses before future integration.
