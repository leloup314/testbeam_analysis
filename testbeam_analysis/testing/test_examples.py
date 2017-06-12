''' Script to check that the examples run.

    The example data is reduced at the beginning to safe time.
'''
import os
import unittest
import mock
from shutil import copyfile
import tables as tb

import testbeam_analysis
from testbeam_analysis.tools import data_selection
from testbeam_analysis.examples import (eutelescope,
                                        fei4_telescope,
                                        simulated_data)

# Get the absoulte path of the online_monitor installation
testing_path = os.path.dirname(__file__)
package_path = os.path.dirname(testbeam_analysis.__file__)
script_folder = os.path.abspath(os.path.join(package_path, r'examples/'))
fixture_folder = os.path.abspath(os.path.join(os.path.dirname(
    os.path.realpath(testing_path)) + r'/testing/fixtures/examples/'))
tests_data_folder = os.path.abspath(
    os.path.join(os.path.realpath(script_folder), r'data/'))


def copy_alignment(path, out_folder, **kwarg):
    try:
        os.mkdir(os.path.join(tests_data_folder, out_folder))
    except OSError:
        pass
    copyfile(os.path.join(fixture_folder, path),
             os.path.join(tests_data_folder,
                          os.path.join(out_folder, 'Alignment.h5')))


# Wrapps the original fit tracks call to reduce the number of fitted DUTs
# when using Kalman filter to save time
orig_fit_track = testbeam_analysis.track_analysis.fit_tracks


def fit_tracks_fast(input_track_candidates_file, input_alignment_file, output_tracks_file, fit_duts=None,
                    selection_hit_duts=None, selection_fit_duts=None, exclude_dut_hit=True, selection_track_quality=1,
                    pixel_size=None, n_pixels=None, beam_energy=None, material_budget=None, add_scattering_plane=None,
                    max_tracks=None, use_prealignment=False, use_correlated=False, min_track_distance=False, keep_data=False, method='Fit',
                    full_track_info=False, chunk_size=1000000):
    orig_fit_track(
        input_track_candidates_file,
        input_alignment_file,
        output_tracks_file,
        fit_duts=fit_duts if method == 'Fit' else [2],
        selection_hit_duts=selection_hit_duts,
        selection_fit_duts=selection_fit_duts,
        exclude_dut_hit=exclude_dut_hit,
        selection_track_quality=selection_track_quality,
        pixel_size=pixel_size,
        n_pixels=n_pixels,
        beam_energy=beam_energy,
        material_budget=material_budget,
        add_scattering_plane=add_scattering_plane,
        max_tracks=max_tracks,
        use_prealignment=use_prealignment,
        use_correlated=use_correlated,
        min_track_distance=min_track_distance,
        keep_data=keep_data,
        method=method,
        full_track_info=full_track_info,
        chunk_size=chunk_size)


class TestExamples(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Virtual X server for plots under headless LINUX travis testing needed
        if os.getenv('TRAVIS', False):
            from xvfbwrapper import Xvfb
            cls.vdisplay = Xvfb()
            cls.vdisplay.start()

        cls.output_folder = tests_data_folder

        # Reduce the example data to make it possible to test the examples in
        # CI environments
        cls.examples_fei4_hit_files = [os.path.join(
            cls.output_folder,
            r'TestBeamData_FEI4_DUT0.h5'),
            os.path.join(
            cls.output_folder,
            r'TestBeamData_FEI4_DUT1.h5'),
            os.path.join(
            cls.output_folder,
            r'TestBeamData_FEI4_DUT4.h5'),
            os.path.join(
            cls.output_folder,
            r'TestBeamData_FEI4_DUT5.h5')]
        data_selection.reduce_hit_files(
            cls.examples_fei4_hit_files, fraction=100)

        cls.examples_mimosa_hit_files = [os.path.join(
            cls.output_folder,
            r'TestBeamData_Mimosa26_DUT%d.h5') % i for i in range(6)]
        data_selection.reduce_hit_files(
            cls.examples_mimosa_hit_files, fraction=100)

        # Remove old files and rename reduced files
        for file_name in cls.examples_fei4_hit_files:
            os.remove(file_name)
            os.rename(os.path.splitext(file_name)[0] + '_reduced.h5',
                      file_name)
        for file_name in cls.examples_mimosa_hit_files:
            os.remove(file_name)
            os.rename(os.path.splitext(file_name)[0] + '_reduced.h5',
                      file_name)

    # Alignments do not converge for reduced data set
    # Thus mock out the alignment steps
    @mock.patch('testbeam_analysis.dut_alignment.prealignment',
                side_effect=copy_alignment(
                    path=r'eutelescope/Alignment.h5',
                    out_folder=r'output_eutel')
                )
    @mock.patch('testbeam_analysis.dut_alignment.alignment')
    # TODO: Analysis fails, to be checked why
    @mock.patch('testbeam_analysis.track_analysis.fit_tracks',
                side_effect=fit_tracks_fast)
    @mock.patch('testbeam_analysis.result_analysis.calculate_residuals')
    def test_mimosa_example(self, m1, m2, m3, m4):
        eutelescope.run_analysis()

    # Prealignment does not converge for reduced data set
    # Thus mock out the prealignment
    @mock.patch('testbeam_analysis.dut_alignment.prealignment',
                side_effect=copy_alignment(
                    path=r'fei4_telescope/Alignment.h5',
                    out_folder=r'output_fei4'),
                )
    def test_fei4_example(self, mock):
        fei4_telescope.run_analysis()

    def test_simulated_data_example(self):
        ''' Check the example and the overall analysis that a efficiency of
            about 100% is reached. Not a perfect 100% is expected due to the
            finite propability that tracks are merged since > 2 tracks per
            event are simulated
        '''
        simulated_data.run_analysis(1000)
        with tb.open_file('simulation/Efficiency.h5') as in_file_h5:
            for i in range(5):  # Loop over DUT index
                eff = in_file_h5.get_node('/DUT_%d/Efficiency' % i)[:]
                mask = in_file_h5.get_node('/DUT_%d/Efficiency_mask' % i)[:]
                self.assertAlmostEqual(eff[~mask].mean(), 100., delta=0.0001)


if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestExamples)
    unittest.TextTestRunner(verbosity=2).run(suite)
