/*
		Name: KINEMATIC_LINEAR_SCARA.c
		Description: Implements all kinematics math equations to translate the motion of a scara machine.
				Also implements the homing motion for this type of machine.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 28/072023

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_6DOF_ARM)

#include <stdint.h>
#include <math.h>

// Threshold for convergence in inverse kinematics
#define IK_TOLERANCE 1e-6
#define IK_MAX_ITERATIONS 1000

// Structure to hold robot parameters
typedef struct
{
	float a[AXIS_TO_STEPPERS];						// Link lengths along x-axis (meters)
	float alpha[AXIS_TO_STEPPERS];				// Link twists around x-axis (radians)
	float d[AXIS_TO_STEPPERS];						// Link offsets along z-axis (meters)
	float theta_offset[AXIS_TO_STEPPERS]; // Joint angle offsets (radians)
	float joint_min[AXIS_TO_STEPPERS];		// Minimum joint angles (degrees)
	float joint_max[AXIS_TO_STEPPERS];		// Maximum joint angles (degrees)
} RobotParameters;

static RobotParameters robotParam;

void kinematics_init(void)
{
	// reset home offset
	memcpy(robotParam.a, g_settings.dof_a, sizeof(robotParam.a));
	memcpy(robotParam.alpha, g_settings.dof_alpha, sizeof(robotParam.alpha));
	memcpy(robotParam.d, g_settings.dof_d, sizeof(robotParam.d));
	memcpy(robotParam.theta_offset, g_settings.dof_theta_offset, sizeof(robotParam.theta_offset));
	memcpy(robotParam.joint_min, g_settings.dof_joint_min, sizeof(robotParam.joint_min));
	memcpy(robotParam.joint_max, g_settings.dof_joint_max, sizeof(robotParam.joint_max));
	mc_sync_position();
}

static void computeDHMatrix(float theta, float d, float a, float alpha, float T[4][4])
{
	// Angles are in radians
	float ct = cosf(theta);
	float st = sinf(theta);
	float ca = cosf(alpha);
	float sa = sinf(alpha);

	T[0][0] = ct;
	T[0][1] = -st * ca;
	T[0][2] = st * sa;
	T[0][3] = a * ct;

	T[1][0] = st;
	T[1][1] = ct * ca;
	T[1][2] = -ct * sa;
	T[1][3] = a * st;

	T[2][0] = 0.0f;
	T[2][1] = sa;
	T[2][2] = ca;
	T[2][3] = d;

	T[3][0] = 0.0f;
	T[3][1] = 0.0f;
	T[3][2] = 0.0f;
	T[3][3] = 1.0f;
}

void multiplyMatrices(float A[4][4], float B[4][4], float result[4][4])
{
	float temp[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp[i][j] = 0.0f;
			for (int k = 0; k < 4; k++)
			{
				temp[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	// Copy result back to result matrix
	memcpy(result, temp, sizeof(float) * 16);
}

static void identityMatrix(float T[4][4])
{
	memset(T, 0, sizeof(float) * 16);
	T[0][0] = 1.0f;
	T[1][1] = 1.0f;
	T[2][2] = 1.0f;
	T[3][3] = 1.0f;
}

static void computeForwardKinematics(float jointAngles[AXIS_TO_STEPPERS], float T0_6[4][4], RobotParameters *params)
{
	identityMatrix(T0_6);

	for (int i = 0; i < AXIS_TO_STEPPERS; i++)
	{
		// Convert joint angle to radians
		float theta = (jointAngles[i] + params->theta_offset[i]) * M_PI / 180.0f;
		float d = params->d[i];
		float a = params->a[i];
		float alpha = params->alpha[i]; // Already in radians

		float Ti[4][4];
		computeDHMatrix(theta, d, a, alpha, Ti);

		multiplyMatrices(T0_6, Ti, T0_6); // T0_6 = T0_6 * Ti
	}
}

static int computeInverseKinematics(float desiredPose[AXIS_COUNT], float jointAngles[AXIS_TO_STEPPERS], RobotParameters *params)
{
	float lambda = 0.01f; // Initial damping factor
	float nu = 2.0f;			// Factor to adjust lambda
	float deltaTheta[AXIS_TO_STEPPERS];
	float currentPose[AXIS_COUNT];
	float error[AXIS_COUNT];
	float J[AXIS_COUNT][AXIS_TO_STEPPERS];
	float J_T[AXIS_TO_STEPPERS][AXIS_COUNT];
	float A[AXIS_TO_STEPPERS][AXIS_TO_STEPPERS];
	float b[AXIS_TO_STEPPERS];

	for (int iter = 0; iter < IK_MAX_ITERATIONS; iter++)
	{
		// Compute current end-effector pose
		float T0_6[4][4];
		computeForwardKinematics(jointAngles, T0_6, params);
		extractPoseFromMatrix(T0_6, currentPose);

		// Compute error between desired and current pose
		for (int i = 0; i < AXIS_COUNT; i++)
		{
			error[i] = desiredPose[i] - currentPose[i];
			// Normalize angle differences to [-180, 180]
			if (i >= 3)
			{
				while (error[i] > 180.0f)
					error[i] -= 360.0f;
				while (error[i] < -180.0f)
					error[i] += 360.0f;
				// Convert to radians
				error[i] = error[i] * M_PI / 180.0f;
			}
		}

		// Compute error norm
		float errorNorm = 0.0f;
		for (int i = 0; i < 6; i++)
		{
			errorNorm += error[i] * error[i];
		}
		errorNorm = sqrtf(errorNorm);

		// Check for convergence
		if (errorNorm < IK_TOLERANCE)
		{
			return 1; // Solution found
		}

		// Compute Jacobian
		computeJacobian(jointAngles, J, params);

		// Compute J^T
		for (int i = 0; i < AXIS_TO_STEPPERS; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				J_T[i][j] = J[j][i];
			}
		}

		// Compute A = J^T * J + lambda * I
		for (int i = 0; i < AXIS_TO_STEPPERS; i++)
		{
			for (int j = 0; j < AXIS_TO_STEPPERS; j++)
			{
				A[i][j] = 0.0f;
				for (int k = 0; k < 6; k++)
				{
					A[i][j] += J_T[i][k] * J[k][j];
				}
				if (i == j)
				{
					A[i][j] += lambda;
				}
			}
		}

		// Compute b = J^T * error
		for (int i = 0; i < AXIS_TO_STEPPERS; i++)
		{
			b[i] = 0.0f;
			for (int k = 0; k < 6; k++)
			{
				b[i] += J_T[i][k] * error[k];
			}
		}

		// Solve A * deltaTheta = b
		if (!solveLinearSystem(A, b, deltaTheta, AXIS_TO_STEPPERS))
		{
			// Cannot solve linear system
			return -2; // Singular matrix encountered
		}

		// Tentatively update joint angles
		float tempJointAngles[AXIS_TO_STEPPERS];
		for (int i = 0; i < AXIS_TO_STEPPERS; i++)
		{
			tempJointAngles[i] = jointAngles[i] + deltaTheta[i] * 180.0f / M_PI;
		}

		// Enforce joint limits
		for (int i = 0; i < AXIS_TO_STEPPERS; i++)
		{
			if (tempJointAngles[i] > params->joint_max[i] || tempJointAngles[i] < params->joint_min[i])
			{
				return -1; // Pose unreachable within joint limits
			}
		}

		// Compute new error norm with tentative joint angles
		float T0_6_new[4][4];
		computeForwardKinematics(tempJointAngles, T0_6_new, &robotParam);
		float newPose[6];
		extractPoseFromMatrix(T0_6_new, newPose);

		// Compute new error norm
		float newError[6];
		float newErrorNorm = 0.0f;
		for (int i = 0; i < 6; i++)
		{
			newError[i] = desiredPose[i] - newPose[i];
			if (i >= 3)
			{
				while (newError[i] > 180.0f)
					newError[i] -= 360.0f;
				while (newError[i] < -180.0f)
					newError[i] += 360.0f;
				newError[i] = newError[i] * M_PI / 180.0f; // Convert to radians
			}
			newErrorNorm += newError[i] * newError[i];
		}
		newErrorNorm = sqrtf(newErrorNorm);

		// Adjust lambda based on error norm
		if (newErrorNorm < errorNorm)
		{
			// Error decreased; accept the update
			for (int i = 0; i < AXIS_TO_STEPPERS; i++)
			{
				jointAngles[i] = tempJointAngles[i];
			}
			lambda /= nu; // Decrease lambda
			errorNorm = newErrorNorm;
		}
		else
		{
			// Error increased; reject the update
			lambda *= nu; // Increase lambda
			if (lambda > 1e7f)
			{
				// Lambda too large; convergence unlikely
				return -3; // Failed to converge
			}
		}
	}

	return 0; // Maximum iterations reached without convergence
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
	float jointAngles[AXIS_TO_STEPPERS];

	// convert motor steps to degrees
	for (uint8_t i = 0; i < AXIS_TO_STEPPERS; i++)
	{
		jointAngles[i] = steps[i] * g_settings.step_per_mm[i];
	}

	if (computeInverseKinematics(axis, jointAngles, &robotParam) > 0)
	{
		for (uint8_t i = 0; i < AXIS_TO_STEPPERS; i++)
		{
			steps[i] = jointAngles[i] / g_settings.step_per_mm[i];
		}
	}
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
	float jointAngles[AXIS_TO_STEPPERS];

	// convert motor steps to degrees
	for (uint8_t i = 0; i < AXIS_TO_STEPPERS; i++)
	{
		jointAngles[i] = steps[i] * g_settings.step_per_mm[i];
	}

	float T0_6[4][4];
	computeForwardKinematics(jointAngles, T0_6, &robotParam);
	extractPoseFromMatrix(T0_6, axis);
}

uint8_t kinematics_home(void)
{
	return STATUS_OK;
}

void kinematics_apply_transform(float *axis)
{
}

void kinematics_apply_reverse_transform(float *axis)
{
}

bool kinematics_check_boundaries(float *axis)
{
	return true;
}

#endif
